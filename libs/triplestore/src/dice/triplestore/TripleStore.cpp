#include "TripleStore.hpp"

#include <dice/metall-node-storage/MetallNodeStorageBackend.hpp>
#include <dice/sparql/parser/SPARQLParser.hpp>
#include <dice/logger.hpp>

#include <rdf4cpp/rdf.hpp>

#include <cerrno>
#include <fstream>

// TODO: remove once rdf4cpp implements formatter for ParsingError
namespace std {
	template<>
	struct formatter<::rdf4cpp::rdf::parser::ParsingError> : formatter<string_view> {
		template<typename FmtCtx>
		auto format(::rdf4cpp::rdf::parser::ParsingError const &err, FmtCtx &ctx) const {
			ostringstream oss;
			oss << err;

			return formatter<string_view>::format(oss.view(), ctx);
		}
	};
} // namespace std

namespace dice::triplestore {
	using ::dice::metall_node_storage::MetallNodeStorageBackend;

	static constexpr char const *context_suffix = "context";
	static constexpr char const *hypertrie_suffix = "hypertrie";

	TripleStoreIterator::reference TripleStoreIterator::operator*() const noexcept {
		return this->iter.value();
	}

	TripleStoreIterator::pointer TripleStoreIterator::operator->() const noexcept {
		return &this->iter.value();
	}

	TripleStoreIterator &TripleStoreIterator::operator++() noexcept {
		this->iter.advance();
		return *this;
	}

	bool TripleStoreIterator::operator==(std::default_sentinel_t) const noexcept {
		return this->iter.ended();
	}

	bool TripleStoreIterator::operator!=(std::default_sentinel_t) const noexcept {
		return !this->iter.ended();
	}

	TripleStore::TripleStore(NodeStorage &&node_storage, defs::HypertrieContext *hypertrie_context, defs::BoolHypertrie *hypertrie) noexcept
		: node_storage_{std::move(node_storage)},
		  hypertrie_context_{hypertrie_context},
		  hypertrie_{hypertrie},
		  inserter_{*hypertrie_} {
	}

	TripleStore::TripleStore(std::tuple<NodeStorage, defs::HypertrieContext *, defs::BoolHypertrie *> params) noexcept
		: TripleStore{std::move(std::get<0>(params)), std::get<1>(params), std::get<2>(params)} {
	}

	TripleStore::TripleStore(defs::InMemoryFlag) noexcept
		: TripleStore{[]() noexcept {
			  allocator_type allocator{defs::std_allocator_type{}};

			  auto *hypertrie_context = new defs::HypertrieContext{allocator};
			  auto *hypertrie = new defs::BoolHypertrie{3, hypertrie_context};

			  return std::make_tuple(NodeStorage::new_instance<MetallNodeStorageBackend>(defs::in_memory),
									 hypertrie_context,
									 hypertrie);
		  }()} {
	}

	TripleStore::TripleStore(defs::PersistentFlag, defs::metall_manager &manager, std::string const &name)
		: TripleStore{[&manager, &name]() {
			  allocator_type allocator{manager.get_allocator()};

			  auto const hypertrie_context_name = name + context_suffix;// TODO: add tentris version?
			  auto const hypertrie_name = name + hypertrie_suffix;
			  auto *hypertrie_context = manager.find_or_construct<defs::HypertrieContext>(hypertrie_context_name.c_str())(allocator);
			  auto *hypertrie = manager.find_or_construct<defs::BoolHypertrie>(hypertrie_name.c_str())(3, hypertrie_context);

			  return std::make_tuple(NodeStorage::new_instance<MetallNodeStorageBackend>(defs::persistent, manager, name),
									 hypertrie_context,
									 hypertrie);
		  }()} {
	}

	TripleStore::~TripleStore() {
		{
			std::unique_lock<std::shared_mutex> writer_lock{mutex_};
			inserter_.flush();
		}
		if (is_in_memory()) {
			delete hypertrie_;
			delete hypertrie_context_;
		}
	}

	void TripleStore::load_ttl(std::string const &file_path,
							   uint32_t bulk_size) {
		std::ifstream ifs{file_path};
		if (!ifs.is_open()) {
			throw std::system_error{std::error_code{errno, std::system_category()}};
		}

		flush();
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};

		auto const call_back = [batch_start = std::chrono::steady_clock::now(), func_name = __PRETTY_FUNCTION__](size_t processed_entries, [[maybe_unused]] size_t committed_entries, size_t hypertrie_size_after) mutable {
			auto batch_end = std::chrono::steady_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(batch_end - batch_start);

			// TODO fmt width
			dice::logger::log(dice::logger::Level::Info, func_name,
							  "Loaded batch {:>10.3} mio triples processed, {:>10.2f}s elapsed, {:>10.2} mio triples in storage",
							  static_cast<double>(processed_entries) / 1'000'000.0,
							  elapsed.count(),
							  static_cast<double>(hypertrie_size_after) / 1'000'000.0);

			batch_start = batch_end;
		};

		defs::HypertrieAsyncBulkInserter bulk_inserter{*hypertrie_, bulk_size, call_back};

		using namespace rdf4cpp::rdf::parser;

		IStreamQuadIterator qit{ifs,
								ParsingFlags::none(),
								IStreamQuadIterator::prefix_storage_type{},
								node_storage_};
		for (; qit != IStreamQuadIterator{}; ++qit) {

			if (qit->has_value()) {
				auto const &quad = qit->value();
				bulk_inserter.add(hypertrie::internal::raw::SingleEntry<3, defs::htt_t>{{quad.subject(),
																						 quad.predicate(),
																						 quad.object()},
																						true});
			} else {
				TENTRIS_WARN("{}", qit->error());
			}
		}
	}

	void TripleStore::add_statement(rdf4cpp::rdf::Statement const &statement) {
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		hypertrie::internal::raw::SingleEntry<3, defs::htt_t> entry{{statement.subject().to_node_storage(node_storage_),
																	 statement.predicate().to_node_storage(node_storage_),
																	 statement.object().to_node_storage(node_storage_)},
																	true};
		inserter_.add(entry);
	}

	bool TripleStore::is_rdf_list(rdf4cpp::rdf::Node list) const noexcept {
		flush();

		if (list.backend_handle().node_storage_id() != node_storage_.id()) {
			return false; // not a list in this storage
		}

		rdf4cpp::rdf::IRI const rdf_list_nil{"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"};
		rdf4cpp::rdf::IRI const rdf_list_first{"http://www.w3.org/1999/02/22-rdf-syntax-ns#first"};
		rdf4cpp::rdf::IRI const rdf_list_rest{"http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"};

		std::shared_lock<std::shared_mutex> reader_lock{mutex_};

		if (list == rdf_list_nil) {
			return true; // empty collection
		}

		auto const prop_obj = hypertrie()[defs::SliceKey{list, std::nullopt, std::nullopt}];
		if (prop_obj.empty()) {
			return false;
		}

		if (auto const has_first = prop_obj[defs::SliceKey{rdf_list_first, std::nullopt}]; has_first.size() != 1) {
			return false;
		}

		if (auto const has_rest = prop_obj[defs::SliceKey{rdf_list_rest, std::nullopt}]; has_rest.size() != 1) {
			return false;
		}

		return true;
	}

	std::vector<rdf4cpp::rdf::Node> TripleStore::get_rdf_list(rdf4cpp::rdf::Node list) const {
		flush();

		rdf4cpp::rdf::IRI const rdf_list_nil{"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"};
		rdf4cpp::rdf::IRI const rdf_list_first{"http://www.w3.org/1999/02/22-rdf-syntax-ns#first"};
		rdf4cpp::rdf::IRI const rdf_list_rest{"http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"};

		std::vector<rdf4cpp::rdf::Node> node_vector;
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};

		auto head = list;
		while (head != rdf_list_nil) {
			auto const element = hypertrie()[defs::SliceKey{list, rdf_list_first, std::nullopt}];

			if (auto const element_sz = element.size(); element_sz == 0) [[unlikely]] {
				throw std::runtime_error{"Invalid RDF seq. No first elements for list node {}" + std::string(head)};
			} else if (element_sz > 1) [[unlikely]] {
				throw std::runtime_error{"Invalid RDF seq. Multiple first elements for list node {}" + std::string(head)};
			}

			node_vector.push_back((*element.begin())[0]);

			auto const rest = hypertrie()[defs::SliceKey{list, rdf_list_rest, std::nullopt}];
			if (auto const rest_sz = rest.size(); rest_sz == 0) {
				break; // this is not canonical but seems better than throwing an error
			} else if (rest_sz > 1) [[unlikely]] {
				throw std::runtime_error{"Invalid RDF seq. Multiple rest elements for list node {}" + std::string(head)};
			}

			head = (*element.begin())[0];
		}

		return node_vector;
	}

	TripleStore::SolutionMappingGenerator
	TripleStore::eval_sparql_query(sparql::SPARQLQuery const &sparql_query, std::chrono::steady_clock::time_point end_time) const {
		flush();
		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		sparql::detail::EvaluationContext eval_ctx{hypertrie(), end_time};

		auto raw_query = sparql_query.raw_query();
		switch (sparql_query.query_type()) {
			case sparql::SPARQLQuery::QueryType::DESCRIBE: {
				assert(false);
				break;
			}
			case sparql::SPARQLQuery::QueryType::CONSTRUCT: {
				for (auto const &solution : query::Evaluation::evaluate(raw_query, eval_ctx)) {
					// The blank node labels are scoped to the template for each solution
					boost::container::flat_map<rdf4cpp::rdf::query::Variable, rdf4cpp::rdf::BlankNode> construct_bnode_to_id{};
					for (auto const &triple_pattern : sparql_query.construct_template()) {
						auto construct_entry = sparql::detail::Entry::make_with_defaulted_key(3, 1);
						size_t i = 0;
						bool skip = false;
						for (auto const &term : triple_pattern) {
							if (term.is_variable()) {
								if (term.as_variable().is_anonymous()) {
									auto bnode = term.as_variable();
									if (auto found = construct_bnode_to_id.find(bnode); found == construct_bnode_to_id.end()) {
										auto bnode_id = rdf4cpp::rdf::BlankNode::make(std::to_string(bnode_id_++));
										construct_bnode_to_id[bnode] = bnode_id;
										construct_entry.key()[i] = bnode_id;
									} else {
										construct_entry.key()[i] = found->second;
									}
								} else {
									construct_entry.key()[i] = solution[sparql_query.quad_template_variable_position(term.as_variable())];
								}
							} else {
								construct_entry.key()[i] = term;
							}
							if (construct_entry.key()[i].null()) {
								skip = true;
								break;
							}
							i++;
						}
						if (skip)
							continue;
						co_yield construct_entry;
					}
				}
				break;
			}
			// select or ask. update operations that have a where clause also enter this part
			default: {
				co_yield std::elements_of(query::Evaluation::evaluate(raw_query, eval_ctx));
			}
		}
	}

	void
	TripleStore::eval_sparql_update(sparql::SPARQLQuery const &sparql_update_query, std::chrono::steady_clock::time_point end_time) {
		// todo: when multiple operations are supported we will have to update the EvaluationContext after each operation

		flush();

		std::vector<hypertrie::internal::raw::SingleEntry<3, tentris::defs::htt_t>> entries_to_remove{};
		std::vector<hypertrie::internal::raw::SingleEntry<3, tentris::defs::htt_t>> entries_to_insert{};
		// DELETE DATA
		if (sparql_update_query.query_type() == sparql::SPARQLQuery::QueryType::DELETE_DATA) {
			for (auto const &quad : sparql_update_query.delete_template()) {
				hypertrie::internal::raw::SingleEntry<3, tentris::defs::htt_t> entry{{quad.subject().try_get_in_node_storage(node_storage_),
																					  quad.predicate().try_get_in_node_storage(node_storage_),
																					  quad.object().try_get_in_node_storage(node_storage_)}};
				// the bulk remover ensures that the entries that are passed for removal exist in the graph
				entries_to_remove.push_back(entry);
			}
		}
		// INSERT DATA
		else if (sparql_update_query.query_type() == sparql::SPARQLQuery::QueryType::INSERT_DATA) {
			for (auto const &quad : sparql_update_query.insert_template()) {
				hypertrie::internal::raw::SingleEntry<3, tentris::defs::htt_t> entry{{quad.subject().to_node_storage(node_storage_),
																					  quad.predicate().to_node_storage(node_storage_),
																					  quad.object().to_node_storage(node_storage_)}};
				entries_to_insert.push_back(entry);
			}
		}
		// DELETE WHERE {}, DELETE {} WHERE {}, DELETE {} INSERT {} WHERE {}
		else {
			if (sparql_update_query.query_type() == sparql::SPARQLQuery::QueryType::DELETE) {
				// we need to evaluate the graph pattern and then instantiate the templates
				for (auto const &result_entry : eval_sparql_query(sparql_update_query, end_time)) {
					populate_entries_for_update_op<UPDATE_OP::DELETE>(sparql_update_query,
																	  entries_to_remove,
																	  result_entry);
				}
			} else if (sparql_update_query.query_type() == sparql::SPARQLQuery::QueryType::INSERT) {
				// we need to evaluate the graph pattern and then instantiate the templates
				for (auto const &result_entry : eval_sparql_query(sparql_update_query, end_time)) {
					populate_entries_for_update_op<UPDATE_OP::INSERT>(sparql_update_query,
																	  entries_to_insert,
																	  result_entry);
				}
			} else {
				// we need to evaluate the graph pattern and then instantiate the templates
				for (auto const &result_entry : eval_sparql_query(sparql_update_query, end_time)) {
					populate_entries_for_update_op<UPDATE_OP::DELETE>(sparql_update_query,
																	  entries_to_remove,
																	  result_entry);
					populate_entries_for_update_op<UPDATE_OP::INSERT>(sparql_update_query,
																	  entries_to_insert,
																	  result_entry);
				}
			}
		}
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		// bulk removal
		{
			tentris::defs::HypertrieSyncBulkRemover bulk_remover{*hypertrie_};
			for (auto const &to_rem : entries_to_remove) {
				bulk_remover.add(to_rem.key());
			}
		}
		// bulk insertion
		{
			tentris::defs::HypertrieSyncBulkInserter bulk_inserter{*hypertrie_};
			for (auto const &to_ins : entries_to_insert) {
				bulk_inserter.add(to_ins);
			}
		}
	}

	template<TripleStore::UPDATE_OP op>
	void TripleStore::populate_entries_for_update_op(sparql::SPARQLQuery const &sparql_update_query,
													 std::vector<hypertrie::internal::raw::SingleEntry<3, tentris::defs::htt_t>> &update_entries,
													 sparql::detail::Entry const &result_entry) {
		auto const &quad_template = [&]() -> std::vector<rdf4cpp::rdf::query::QuadPattern> const & {
			if constexpr (op == TripleStore::UPDATE_OP::DELETE)
				return sparql_update_query.delete_template();
			else
				return sparql_update_query.insert_template();
		};
		// The blank node labels are scoped to the template for each solution
		boost::container::flat_map<rdf4cpp::rdf::query::Variable, rdf4cpp::rdf::BlankNode> bnode_to_id{};
		for (auto const &quad_pattern : quad_template()) {
			hypertrie::internal::raw::SingleEntry<3, sparql::detail::htt_t> entry{};
			size_t i = 0;
			bool skip = false;
			// skip graph position
			for (auto iter = quad_pattern.begin() + 1; iter != quad_pattern.end(); iter++) {
				auto term = *iter;
				if (term.is_variable()) {
					if constexpr (op == TripleStore::UPDATE_OP::DELETE) {
						entry.key()[i] = result_entry[sparql_update_query.quad_template_variable_position(term.as_variable())];
					} else {// op == TripleStore::UPDATE_OP::INSERT
						if (term.as_variable().is_anonymous()) {
							auto bnode = term.as_variable();
							if (auto found = bnode_to_id.find(bnode); found == bnode_to_id.end()) {
								auto bnode_id = rdf4cpp::rdf::BlankNode::make(std::to_string(bnode_id_++));
								bnode_to_id[bnode] = bnode_id;
								entry.key()[i] = bnode_id;
							} else {
								entry.key()[i] = found->second;
							}
						} else {// normal var; not anonymous
							entry.key()[i] = result_entry[sparql_update_query.quad_template_variable_position(term.as_variable())];
						}
					}
				} else {
					if constexpr (op == TripleStore::UPDATE_OP::DELETE)
						entry.key()[i] = term.try_get_in_node_storage(node_storage_);
					else// op == TripleStore::UPDATE_OP::INSERT
						entry.key()[i] = term.to_node_storage(node_storage_);
				}
				if (entry.key()[i].null()) {
					skip = true;
					break;
				}
				i++;
			}
			if (skip)
				continue;
			update_entries.push_back(entry);
		}
	}

	std::shared_ptr<const sparql::SPARQLQuery>
	TripleStore::parse_sparql_query(std::string const &sparql_query_str) const {
		using SPARQLParser = dice::sparql::parser::SPARQLParser;
		auto sparql_query = sparql_cache_[sparql_query_str];
		if (not sparql_query)
			sparql_query = sparql_cache_.insert(sparql_query_str, SPARQLParser::parse_query(sparql_query_str, node_storage_));
		return sparql_query;
	}

	std::vector<sparql::SPARQLQuery>
	TripleStore::parse_sparql_update(std::string const &sparql_update_str) const {
		using SPARQLParser = dice::sparql::parser::SPARQLParser;
		return SPARQLParser::parse_update(sparql_update_str, node_storage_);
	}

	sparql::SPARQLQuery
	TripleStore::parse_sparql_insert_or_delete_data(std::string const &sparql_update_str) const {
		using SPARQLParser = dice::sparql::parser::SPARQLParser;
		return SPARQLParser::parse_delete_or_insert_data(sparql_update_str);
	}

	rdf4cpp::rdf::Node TripleStore::try_get(rdf4cpp::rdf::Node const resource) const noexcept {
		return resource.try_get_in_node_storage(node_storage_);
	}

	bool TripleStore::contains(rdf4cpp::rdf::Statement const &statement) const {
		auto const r2kp = [this](rdf4cpp::rdf::Node const resource) noexcept -> rdf4cpp::rdf::Node {
			// access with a null resource returned from `try_get_in_node_storage` will
			// result in false being returned from hypertrie::operator[]
			return resource.try_get_in_node_storage(node_storage_);
		};

		hypertrie::internal::raw::RawKey<3, defs::htt_t> const key{r2kp(statement.subject()),
																   r2kp(statement.predicate()),
																   r2kp(statement.object())};

		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		return hypertrie()[key];
	}

	defs::const_BoolHypertrie
	TripleStore::match(rdf4cpp::rdf::query::TriplePattern const &tp) const {
		auto const r2skp = [this](rdf4cpp::rdf::Node resource) noexcept -> std::optional<rdf4cpp::rdf::Node> {
			// slicing with a null resource returned from `try_get_in_node_storage` will
			// result in the empty hypertrie being returned, which is what we want
			return resource.null() ? std::nullopt : std::optional<rdf4cpp::rdf::Node>{resource.try_get_in_node_storage(node_storage_)};
		};

		defs::SliceKey const slice_key{r2skp(tp.subject()),
									   r2skp(tp.predicate()),
									   r2skp(tp.object())};

		std::shared_lock<std::shared_mutex> reader_lock{mutex_};
		return hypertrie()[slice_key];
	}

	size_t TripleStore::size() const {
		flush();

		std::shared_lock<std::shared_mutex> read_lock{mutex_};
		return hypertrie().size();
	}

	TripleStore::iterator TripleStore::begin() const noexcept {
		flush();

		std::shared_lock<std::shared_mutex> read_lock{mutex_};
		dice::hypertrie::internal::raw::RawIterator<3, true, defs::htt_t, defs::allocator_type> iter{static_cast<dice::hypertrie::internal::raw::NodePtr<3, defs::htt_t, defs::allocator_type>>(hypertrie_->raw_node_ptr())};
		return TripleStoreIterator{std::move(read_lock), std::move(iter)};
	}

	TripleStore::sentinel TripleStore::end() const noexcept {
		return std::default_sentinel;
	}

	void TripleStore::flush() const {
		std::unique_lock<std::shared_mutex> writer_lock{mutex_};
		inserter_.flush();
	}

	bool TripleStore::is_in_memory() const noexcept {
		return hypertrie_context_->get_allocator().holds_allocator<defs::std_allocator_type_t>();
	}

	bool TripleStore::is_persistent() const noexcept {
		return hypertrie_context_->get_allocator().holds_allocator<defs::metall_allocator_type_t>();
	}

	bool TripleStore::destroy_persistent(defs::metall_manager &manager, const std::string &name) {
		auto const hypertrie_context_name = name + context_suffix;// TODO: add tentris version?
		auto const hypertrie_name = name + hypertrie_suffix;

		return manager.destroy<defs::BoolHypertrie>(hypertrie_name.c_str())
		        && manager.destroy<defs::HypertrieContext>(hypertrie_context_name.c_str())
				&& MetallNodeStorageBackend::destroy_persistent(manager, name);
	}
}// namespace dice::triplestore
