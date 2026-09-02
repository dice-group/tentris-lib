#ifndef TENTRIS_STORE_TRIPLESTORE
#define TENTRIS_STORE_TRIPLESTORE

#include <dice/sparql/detail/tensor.hpp>
#include <dice/triplestore/SyncedLRUCache.hpp>

#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif
#include <metall/metall.hpp>

#include <dice/sparql/SPARQLQuery.hpp>
#include <dice/sparql/parser/exception/Exceptions.hpp>
#include <shared_mutex>

#include <rdf4cpp/rdf/Statement.hpp>

namespace dice::triplestore {
	namespace defs {
		using namespace ::dice::tentris::defs;
	}

	struct TripleStoreIterator {
		friend class TripleStore;
	private:
		std::shared_lock<std::shared_mutex> triplestore_lock;
		dice::hypertrie::internal::raw::RawIterator<3, true, defs::htt_t, defs::allocator_type> iter;

		TripleStoreIterator(std::shared_lock<std::shared_mutex> lock, dice::hypertrie::internal::raw::RawIterator<3, true, defs::htt_t, defs::allocator_type> iter) noexcept : triplestore_lock{std::move(lock)},
																																											   iter{std::move(iter)} {
		}
	public:
		using value_type = dice::hypertrie::internal::raw::SingleEntry<3, defs::htt_t>;
		using reference = value_type const &;
		using pointer = value_type const *;

		TripleStoreIterator(TripleStoreIterator const &other) noexcept = delete;
		TripleStoreIterator(TripleStoreIterator &&other) noexcept = default;
		TripleStoreIterator &operator=(TripleStoreIterator const &other) noexcept = delete;
		TripleStoreIterator &operator=(TripleStoreIterator &&other) noexcept = default;

		~TripleStoreIterator() noexcept = default;

		reference operator*() const noexcept;
		pointer operator->() const noexcept;

		TripleStoreIterator &operator++() noexcept;

		bool operator==(std::default_sentinel_t) const noexcept;
		bool operator!=(std::default_sentinel_t) const noexcept;
	};

	class TripleStore {
		using SPARQLQueryCache = SyncedLRUCache<std::string, sparql::SPARQLQuery>;

	public:
		using allocator_type = sparql::detail::allocator_type;
		using SolutionMappingGenerator = std::generator<sparql::detail::Entry const &>;
		using NodeStorage = rdf4cpp::rdf::storage::node::NodeStorage;

	protected:
		NodeStorage node_storage_;
		defs::HypertrieContext *hypertrie_context_;
		defs::BoolHypertrie *hypertrie_;
		mutable std::shared_mutex mutex_;
		mutable defs::HypertrieSyncBulkInserter inserter_;
		mutable SPARQLQueryCache sparql_cache_;
		// it used to provide a unique bnode identifier to blank nodes that are to be inserted in the graph by INSERT operations
		// todo: need to fix possible clashes with bnodes that use simple integers and may exist in the graph
		// it is also used in CONSTRUCT queries to create blank nodes found in the CONSTRUCT template
		mutable uint64_t bnode_id_ = 0;

		TripleStore(NodeStorage &&node_storage, defs::HypertrieContext *hypertrie_context, defs::BoolHypertrie *hypertrie) noexcept;
		explicit TripleStore(std::tuple<NodeStorage, defs::HypertrieContext *, defs::BoolHypertrie *> params) noexcept;

	public:
		/**
		 * @brief Makes an in-memory instance of TripleStore.
		 * @details All resources will be freed upon calling the destructor.
		 * @return in-memory instance
		 */
		explicit TripleStore(defs::InMemoryFlag) noexcept;

		/**
		 * @brief Finds or constructs a persistent TripleStore instance.
		 * @details
		 * @note If creation fails, an exception will be thrown. No memory is allocated in then.
		 * @note The lifetime of the storage is managed by the manager that is passed as argument.
		 * @note Only one instance per name must be exist at any point in time.
		 * @param manager The metall_manager used to find or construct the instance
		 * @param name The name of the TripleStore
		 * @return persistent TripleStore instance
		 */
		TripleStore(defs::PersistentFlag, defs::metall_manager &manager, std::string const &name);

		TripleStore(TripleStore const &other) = delete;
		TripleStore &operator=(TripleStore const &other) = delete;

		TripleStore(TripleStore &&other) = delete;
		TripleStore &operator=(TripleStore &&other) = delete;

		~TripleStore();

		/**
		 * @return const ref to internal hypertrie
		 * @warning this function does not lock or flush the triplestore, using this is pretty much always UB
		 * @todo this is UB, lock not taken and held
		 */
		[[nodiscard]] inline defs::BoolHypertrie const &hypertrie() const noexcept {
			return *hypertrie_;
		}

		/**
		 * This function enforces stricter requirements upon rdf:Lists than described in <a href="https://www.w3.org/TR/2014/REC-rdf11-mt-20140225/#rdf-containers">D.3 RDF collections</a>.
		 * An rdf:List must either be the IRI rdf:nil or must have the properties rdf:first and rdf:rest, both with cardinality 1.
		 * @param list the node to be checked if it is a list
		 * @return if list is an rdf:List
		 */
		[[nodiscard]] bool is_rdf_list(rdf4cpp::rdf::Node list) const noexcept;

		/**
		 * Returns the items of an rdf:List as vector.
		 *
		 * Restrictions from is_rdf_list(rdf4cpp::rdf::Node) const noexcept apply.
		 *
		 * @param list the start node of the list
		 * @return the elements of the list as vector
		 * @throws std::runtime_error If the list is malformed.
		 */
		std::vector<rdf4cpp::rdf::Node> get_rdf_list(rdf4cpp::rdf::Node list) const;

		/**
		 * @brief Loads the given ttl file into storage
		 * @param file_path path to a ttl file
		 * @param bulk_size the number of triples to insert to insert per bulk
		 * @param call_back
		 * @throws std::system_error if there is any problem reading the given file
		 */
		void load_ttl(std::string const &file_path,
					  uint32_t bulk_size = 1'000'000);

		void add_statement(rdf4cpp::rdf::Statement const &statement);

		/**
		 * @brief Evaluation of SPARQL queries.
		 * @param query The SPARQL query.
		 * @return A generator yielding the solutions of the query or bool (for ASK)
		 * @throws query::query_timeout on timeout (Might also not throw directly on timeout here and only once you use the generator. This seems to be compiler dependent)
		 * @note the returned generator might throw query::query_timeout on resume
		 */
		SolutionMappingGenerator
		eval_sparql_query(sparql::SPARQLQuery const &sparql_query, std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::time_point::max()) const;

		/**
		 * @brief Evaluation of SPARQL update queries.
		 * @param query The SPARQL queries corresponding to update operations.
		 * @param eval_ctx The evaluation context
		 */
		void
		eval_sparql_update(sparql::SPARQLQuery const &sparql_update_query, std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::time_point::max());

		/**
		 * @brief Parsing of SPARQL queries. Makes use of caching.
		 * @param query The SPARQL query string.
		 * @param eval_ctx the evaluation context
		 * @return A SPARQL query
		 * @throws sparql::parser::exception::parse_error if the provided query could not be parsed
		 * @throws sparql::parser::exception::unsupported_query if the provided query is not SELECT or ASK
		 * @throws sparql::parser::exception::malformed_query if the provided query is malformed in a non-syntactic way
		 */
		std::shared_ptr<const sparql::SPARQLQuery>
		parse_sparql_query(std::string const &sparql_query_str) const;

		/**
		 * @brief Parsing of SPARQL update queries. No caching is involved.
		 * @param query The SPARQL updated query string.
		 * @return A vector of the parsed queries
		 */
		std::vector<sparql::SPARQLQuery>
		parse_sparql_update(std::string const &sparql_update_str) const;

		/**
		 * @brief Parses a _single_ SPARQL DELETE DATA or INSERT DATA update using a dedicated parser. This is meant to be used for large delete/insert data operations.
		 * @param sparql_update_str The SPARQL update string
		 * @return The parsed query
		 */
		sparql::SPARQLQuery
		parse_sparql_insert_or_delete_data(std::string const &sparql_update_str) const;

		rdf4cpp::rdf::Node try_get(rdf4cpp::rdf::Node resource) const noexcept;
		bool contains(rdf4cpp::rdf::Statement const &statement) const;

		/**
		 * @brief matches triple pattern to triplestore
		 * @warning this function does not properly lock the triplestore (i.e. the return value does not keep the lock), calling this is basically always UB
		 * @todo this is UB, lock not held
		 */
		defs::const_BoolHypertrie
		match(rdf4cpp::rdf::query::TriplePattern const &triple_pattern) const;

		[[nodiscard]] size_t size() const;

		using iterator = TripleStoreIterator;
		using sentinel = std::default_sentinel_t;

		[[nodiscard]] iterator begin() const noexcept;
		[[nodiscard]] sentinel end() const noexcept;

		/**
		 * @brief Flushes and applies pending changes.
		 * @details
		 * @note This does not flush changes to storage. That can only be accomplished via metall_manager.
		 */
		void flush() const;

		/**
		 *
		 * @return If this is in-memory.
		 */
		[[nodiscard]] bool is_in_memory() const noexcept;

		/**
		 *
		 * @return If this is transparently persisted to disc.
		 */
		[[nodiscard]] bool is_persistent() const noexcept;

		/**
		 * @brief Destroys an previously created TripleStore instance.
		 * @details Calls the destructor and frees the memory of TripleStore.
		 * @note
		 * If T's destructor throws:
		 * 1) the exception will be thrown (propagated);
		 * 2) the memory will won't be freed;
		 * 3) the object entry will be still removed from the attributed object directory.
		 * Therefore, it is not recommended to throw exception in a destructor.
		 * @param manager An metall_manager that was used to create the instance.
		 * @param name Name of the instance to be destroyed.
		 * @return Returns false if the object was not destroyed.
		 */
		static bool destroy_persistent(tentris::defs::metall_manager &manager, std::string const &name);

	private:
		enum class UPDATE_OP {
			DELETE,
			INSERT
		};

		template<UPDATE_OP op>
		void populate_entries_for_update_op(sparql::SPARQLQuery const &sparql_update_query,
											std::vector<hypertrie::internal::raw::SingleEntry<3, tentris::defs::htt_t>> &update_entries,
											sparql::detail::Entry const &result_entry);

	};
};    // namespace dice::triplestore
#endif//TENTRIS_STORE_TRIPLESTORE
