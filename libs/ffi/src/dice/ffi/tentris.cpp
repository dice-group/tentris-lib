#include "tentris.h"

#include <dice/ffi/metall_internal.hpp>
#include <dice/ffi/SolutionGeneratorImpl.hpp>

#include <dice/triplestore/TripleStore.hpp>

#ifdef TENTRIS_FFI_WITH_HYPERTRIE
#include <dice/ffi/hypertrie_internal_ffi.hpp>
#endif//TENTRIS_FFI_WITH_HYPERTRIE

#include <cerrno>

using namespace dice::triplestore;

using metall_manager_t = dice::metall_ffi::internal::metall_manager;

char const *tentris_strerror(tentris_error const *error) {
	static thread_local std::string local_buf;

	auto const fmt_parse_error = [](std::ostream &os, tentris_parse_error const &base_data) {
		os << "Parse Error: " << base_data.parser_message << ". At line: " << base_data.line << " and position: " << base_data.col;
	};

	std::ostringstream oss;

	switch (error->discriminant) {
		case TENTRIS_E_INTERNAL: {
			oss << "Unknown internal error";
			break;
		}
		case TENTRIS_E_TIMEOUT: {
			oss << "Query evaluation timed out after " << std::chrono::seconds{error->timeout_data.timeout_duration_s}.count() << " seconds";
			break;
		}
		case TENTRIS_E_PARSE_ERROR: {
			fmt_parse_error(oss, error->parse_data);
			break;
		}
		case TENTRIS_E_UNSUPPORTED_QUERY: {
			fmt_parse_error(oss, error->unsupported_query_data.base);
			break;
		}
		case TENTRIS_E_MALFORMED_QUERY: {
			fmt_parse_error(oss, error->malformed_query_data.base);
			break;
		}
	}

	local_buf = oss.str();
	return local_buf.c_str();
}

void tentris_triplestore_init_in_memory(tentris_triplestore *ts) {
	new (ts) TripleStore{defs::in_memory};
}

tentris_result tentris_triplestore_find_or_construct_persistent(tentris_triplestore *ts, metall_manager *manager, char const *name) {
	try {
		new (ts) TripleStore{defs::persistent, *reinterpret_cast<metall_manager_t *>(manager), name};
		return TENTRIS_SUCCESS;
	} catch (...) {
		errno = ENOMEM;
		return TENTRIS_FAILURE;
	}
}

uint64_t tentris_triplestore_rdf_to_id(tentris_triplestore const *ts_, rdf4cpp_node resource) {
	auto const *ts = reinterpret_cast<TripleStore const *>(ts_);
	auto const h = rdf4cpp::rdf::storage::node::identifier::NodeBackendHandle::from_raw(resource.raw_handle);
	return ts->try_get(*reinterpret_cast<rdf4cpp::rdf::Node const *>(&h)).backend_handle().raw();
}

rdf4cpp_node tentris_triplestore_id_to_rdf(tentris_triplestore const *ts_, uint64_t id) {
	auto const *ts = reinterpret_cast<TripleStore const *>(ts_);
	auto const h = rdf4cpp::rdf::storage::node::identifier::NodeBackendHandle::from_raw(id);
	return rdf4cpp_node{ts->try_get(*reinterpret_cast<rdf4cpp::rdf::Node const *>(&h)).backend_handle().raw()};
}

tentris_result tentris_triplestore_load_ttl(tentris_triplestore *ts, char const *path) {
	try {
		reinterpret_cast<TripleStore *>(ts)->load_ttl(path);
		return TENTRIS_SUCCESS;
	} catch (...) {
		// errno will be set by load_ttl
		return TENTRIS_FAILURE;
	}
}

void tentris_triplestore_add_statement(tentris_triplestore *ts, tentris_triple const *triple) {
	using namespace dice::node_wrapper;
	reinterpret_cast<TripleStore *>(ts)->add_statement(rdf4cpp::rdf::Statement{NodeWrapper{triple->key[0].raw_handle},
																			   NodeWrapper{triple->key[1].raw_handle},
																			   NodeWrapper{triple->key[2].raw_handle}});
}

size_t tentris_triplestore_size(tentris_triplestore const *ts) {
	return reinterpret_cast<TripleStore const *>(ts)->size();
}

void tentris_triplestore_destroy(tentris_triplestore *ts) {
	reinterpret_cast<TripleStore *>(ts)->~TripleStore();
}

tentris_result tentris_triplestore_destroy_persistent(metall_manager *manager, char const *name) {
	if (!TripleStore::destroy_persistent(*reinterpret_cast<metall_manager_t *>(manager), name)) {
		return TENTRIS_FAILURE;
	}

	return TENTRIS_SUCCESS;
}

using namespace dice::node_wrapper;

bool rdf4cpp_term_null(rdf4cpp_node term) {
	using namespace rdf4cpp::rdf;
	return NodeWrapper{term.raw_handle}.null();
}

rdf4cpp_term_type rdf4cpp_type_of_term(rdf4cpp_node term_) {
	using namespace rdf4cpp::rdf::storage::node::identifier;
	static_assert(static_cast<int>(RDFNodeType::BNode) == RDF4CPP_TT_BNODE);
	static_assert(static_cast<int>(RDFNodeType::IRI) == RDF4CPP_TT_IRI);
	static_assert(static_cast<int>(RDFNodeType::Literal) == RDF4CPP_TT_LITERAL);
	static_assert(static_cast<int>(RDFNodeType::Variable) == RDF4CPP_TT_VARIABLE);

	return static_cast<rdf4cpp_term_type>(NodeWrapper{term_.raw_handle}.backend_handle().type());
}

rdf4cpp_literal rdf4cpp_make_literal(char const *lexical, rdf4cpp_iri const *datatype_iri, char const *language) {
	using namespace rdf4cpp::rdf;

	try {
		if (datatype_iri == nullptr) {
			if (language == nullptr || strcmp(language, "") == 0) {
				return rdf4cpp_literal{Literal::make_simple(lexical).backend_handle().raw()};
			}

			return rdf4cpp_literal{Literal::make_lang_tagged(lexical, language).backend_handle().raw()};
		}

		IRI const datatype{storage::node::identifier::NodeBackendHandle::from_raw(datatype_iri->raw_handle)};

		if (datatype.identifier() == datatypes::rdf::LangString::identifier) {
			if (language == nullptr || strcmp(language, "") == 0) {
				errno = EINVAL;
				return rdf4cpp_literal{Literal::make_null().backend_handle().raw()};
			}

			return rdf4cpp_literal{Literal::make_lang_tagged(lexical, language).backend_handle().raw()};
		}

		return rdf4cpp_literal{Literal::make_typed(lexical, datatype).backend_handle().raw()};
	} catch (...) {
		errno = EINVAL;
		return rdf4cpp_literal{Literal::make_null().backend_handle().raw()};
	}
}

rdf4cpp_iri rdf4cpp_make_iri(char const *iri) {
	try {
		return rdf4cpp_iri{rdf4cpp::rdf::IRI{iri}.backend_handle().raw()};
	} catch (...) {
		errno = EINVAL;
		return rdf4cpp_iri{rdf4cpp::rdf::IRI{}.backend_handle().raw()};
	}
}

rdf4cpp_bnode rdf4cpp_make_bnode(char const *identifier) {
	try {
		return rdf4cpp_iri{rdf4cpp::rdf::BlankNode{identifier}.backend_handle().raw()};
	} catch (...) {
		errno = EINVAL;
		return rdf4cpp_iri{rdf4cpp::rdf::BlankNode{}.backend_handle().raw()};
	}
}

rdf4cpp_variable rdf4cpp_make_variable(char const *name) {
	try {
		return rdf4cpp_iri{rdf4cpp::rdf::query::Variable{name}.backend_handle().raw()};
	} catch (...) {
		errno = EINVAL;
		return rdf4cpp_iri{rdf4cpp::rdf::query::Variable{}.backend_handle().raw()};
	}
}

char const *rdf4cpp_iri_identifier(rdf4cpp_iri i) {
	return NodeWrapper{i.raw_handle}.as_iri().identifier().data();
}

char const *rdf4cpp_bnode_identifier(rdf4cpp_bnode bn) {
	return NodeWrapper{bn.raw_handle}.as_blank_node().identifier().data();
}

char const *rdf4cpp_variable_name(rdf4cpp_variable var) {
	return NodeWrapper{var.raw_handle}.as_variable().name().data();
}

rdf4cpp_lexical_form_lifetime rdf4cpp_literal_lexical_form(rdf4cpp_literal lit, char const **buf) {
	using namespace rdf4cpp::rdf::util;
	static thread_local CowString ret_buf{ownership_tag::borrowed, ""};

	ret_buf = NodeWrapper{lit.raw_handle}.as_literal().lexical_form();
	*buf = ret_buf.view().data();

	return ret_buf.is_borrowed() ? RDF4CPP_LFL_STATIC : RDF4CPP_LFL_EPHEMERAL;
}

char const *rdf4cpp_literal_lang(rdf4cpp_literal lit) {
	return NodeWrapper{lit.raw_handle}.as_literal().language_tag().data();
}

rdf4cpp_iri rdf4cpp_literal_datatype(rdf4cpp_literal lit) {
	return rdf4cpp_iri{NodeWrapper{lit.raw_handle}.as_literal().datatype().backend_handle().raw()};
}

void rdf4cpp_serialize(rdf4cpp_node term, byte_sink sink, void *data) {
	// TODO add ability to write into buffer to rdf4cpp
	auto const str = std::string{NodeWrapper{term.raw_handle}};
	sink(data, str.data(), str.size());
}

inline static void populate_parse_error(dice::sparql::parser::exception::parse_error const &e, tentris_error *error) noexcept {
	assert(error != nullptr);

	static thread_local std::string const local_msg_buf = e.parser_msg();

	tentris_parse_error base{.line = e.line(),
							 .col = e.col(),
							 .parser_message = local_msg_buf.c_str()};

	if (auto const *uqe = dynamic_cast<dice::sparql::parser::exception::unsupported_query const *>(&e); uqe != nullptr) {
		static thread_local std::string const local_op_buf = uqe->unsupported_operation();

		*error = tentris_error{.discriminant = TENTRIS_E_UNSUPPORTED_QUERY,
							   .unsupported_query_data = tentris_unsupported_query_error{.base = base,
																						 .unsupported_operation = local_op_buf.c_str()}};
	} else if (dynamic_cast<dice::sparql::parser::exception::malformed_query const *>(&e) != nullptr) {
		*error = tentris_error{.discriminant = TENTRIS_E_MALFORMED_QUERY,
							   .malformed_query_data = tentris_malformed_query_error{.base = base}};
	} else {
		*error = tentris_error{.discriminant = TENTRIS_E_PARSE_ERROR,
							   .parse_data = base};
	}
}

tentris_result tentris_triplestore_eval_query(tentris_triplestore const *ts_, char const *sparql_query_str, unsigned long timeout_ms, tentris_solution_generator *gen_, tentris_error *error) {
	auto const &ts = *reinterpret_cast<TripleStore const *>(ts_);
	auto *gen = reinterpret_cast<SolutionGeneratorImpl *>(gen_);

	new (gen) SolutionGeneratorImpl{.triplestore = &ts,
									.end_time = timeout_ms == TENTRIS_NO_TIMEOUT ? std::chrono::steady_clock::time_point::max()
																				 : std::chrono::steady_clock::now() + std::chrono::milliseconds{timeout_ms}};

	try {
		gen->query = ts.parse_sparql_query(sparql_query_str);
	} catch (dice::sparql::parser::exception::parse_error const &e) {
		errno = EINVAL;
		gen->~SolutionGeneratorImpl();
		if (error != nullptr) {
			populate_parse_error(e, error);
		}
		return TENTRIS_FAILURE;
	} catch (...) {
		errno = ENOTRECOVERABLE;
		gen->~SolutionGeneratorImpl();
		if (error != nullptr) {
			*error = tentris_error{.discriminant = TENTRIS_E_INTERNAL};
		}
		return TENTRIS_FAILURE;
	}

    return TENTRIS_SUCCESS;
}

template<bool use_data_streaming_parser>
inline static tentris_result eval_update_impl(tentris_triplestore *ts_, char const *sparql_update_str, unsigned long timeout_ms, tentris_error *error) noexcept {
	auto &ts = *reinterpret_cast<TripleStore *>(ts_);

	auto end_time = timeout_ms == TENTRIS_NO_TIMEOUT ? std::chrono::steady_clock::time_point::max()
													 : std::chrono::steady_clock::now() + std::chrono::milliseconds{timeout_ms};
	try {
		if constexpr (use_data_streaming_parser) {
			auto update = ts.parse_sparql_insert_or_delete_data(sparql_update_str);

			ts.eval_sparql_update(update, end_time);
			return TENTRIS_SUCCESS;
		} else {
			auto updates = ts.parse_sparql_update(sparql_update_str);
			if (updates.size() > 1) {
				errno = EINVAL;
				if (error != nullptr) {
					// using this syntax to work around bug in clang that prevents the fully designated-initializer-initialized version from compiling inside a template function
					// https://github.com/llvm/llvm-project/issues/65143
					*error = tentris_error{};
					error->discriminant = TENTRIS_E_UNSUPPORTED_QUERY;
					error->unsupported_query_data = tentris_unsupported_query_error{.base = tentris_parse_error{.line = 0, .col = 0, .parser_message = "found composed update"},
																					.unsupported_operation = "composed update"};
				}
				return TENTRIS_FAILURE;
			}

			ts.eval_sparql_update(updates[0], end_time);
			return TENTRIS_SUCCESS;
		}
	} catch (dice::sparql::parser::exception::parse_error const &parse_error) {
		errno = EINVAL;
		if (error != nullptr) {
			populate_parse_error(parse_error, error);
		}
		return TENTRIS_FAILURE;
	} catch (dice::query::query_timeout const &timeout_error) {
		errno = ETIMEDOUT;
		if (error != nullptr) {
			*error = tentris_error{};
			error->discriminant = TENTRIS_E_TIMEOUT;
			error->timeout_data = tentris_timeout_error{.timeout_duration_s = std::chrono::duration_cast<std::chrono::seconds>(timeout_error.timeout_duration()).count()};
		}
		return TENTRIS_FAILURE;
	} catch (...) {
		errno = ENOTRECOVERABLE;
		if (error != nullptr) {
			*error = tentris_error{};
			error->discriminant = TENTRIS_E_INTERNAL;
		}
		return TENTRIS_FAILURE;
	}
}

tentris_result tentris_triplestore_eval_update(tentris_triplestore *ts, char const *sparql_update_str, unsigned long timeout_ms, tentris_error *error) {
	return eval_update_impl<false>(ts, sparql_update_str, timeout_ms, error);
}

tentris_result tentris_triplestore_eval_insert_or_delete_data(tentris_triplestore *ts, char const *sparql_update_str, unsigned long timeout_ms, tentris_error *error) {
	return eval_update_impl<true>(ts, sparql_update_str, timeout_ms, error);
}

tentris_iter_result tentris_solution_generator_next(tentris_solution_generator *gen_, tentris_solution *out_solution, tentris_error *error) {
	auto &gen = *reinterpret_cast<SolutionGeneratorImpl *>(gen_);

	if (!gen.init) {
		gen.init = true;

		try {
			gen.generator = gen.triplestore->eval_sparql_query(*gen.query);
			gen.iter = gen.generator.begin();
		} catch (dice::query::query_timeout const &e) {
			errno = ETIMEDOUT;
			gen.~SolutionGeneratorImpl();
			if (error != nullptr) {
				*error = tentris_error{.discriminant = TENTRIS_E_TIMEOUT,
									   .timeout_data = tentris_timeout_error{.timeout_duration_s = std::chrono::duration_cast<std::chrono::seconds>(e.timeout_duration()).count()}};
			}
			return TENTRIS_I_FAILURE;
		} catch (...) {
			errno = ENOTRECOVERABLE;
			if (error != nullptr) {
				*error = tentris_error{.discriminant = TENTRIS_E_INTERNAL};
			}
			return TENTRIS_I_FAILURE;
		}
	} else {
		try {
			++gen.iter;
		} catch (dice::query::query_timeout const &e) {
			errno = ETIMEDOUT;
			if (error != nullptr) {
				*error = tentris_error{.discriminant = TENTRIS_E_TIMEOUT,
									   .timeout_data = tentris_timeout_error{.timeout_duration_s = std::chrono::duration_cast<std::chrono::seconds>(e.timeout_duration()).count()}};
			}
			return TENTRIS_I_FAILURE;
		}
	}

	if (gen.iter == std::default_sentinel) {
		return TENTRIS_I_ENDED;
	}

	auto const &solution = *gen.iter;
	out_solution->key = reinterpret_cast<rdf4cpp_node const *>(solution.key().as_inner().data());
	out_solution->key_len = solution.size();
	out_solution->count = solution.value();
	return TENTRIS_I_YIELDED;
}

void tentris_solution_generator_get_projected_variables(tentris_solution_generator const *gen_, rdf4cpp_variable const **out_proj_vars, size_t *out_proj_vars_size) {
	auto const &gen = *reinterpret_cast<SolutionGeneratorImpl const *>(gen_);
	*out_proj_vars = reinterpret_cast<rdf4cpp_variable const *>(gen.query->projected_variables().data());
	*out_proj_vars_size = gen.query->projected_variables().size();
}

tentris_query_type tentris_solution_generator_get_query_type(tentris_solution_generator const *gen_) {
	auto const &gen = *reinterpret_cast<SolutionGeneratorImpl const *>(gen_);
	return static_cast<tentris_query_type>(gen.query->query_type());
}

void tentris_solution_generator_destroy(tentris_solution_generator *gen) {
	reinterpret_cast<SolutionGeneratorImpl *>(gen)->~SolutionGeneratorImpl();
}

void tentris_triplestore_iterate(tentris_triplestore const *ts_, tentris_triplestore_iterator *iter) {
	auto const &ts = *reinterpret_cast<TripleStore const *>(ts_);
	new (iter) IteratorImpl{.iter = ts.begin(), .first = true};
}

tentris_iter_result tentris_triplestore_iterator_next(tentris_triplestore_iterator *iter_, tentris_triple const **out_triple) {
	auto &iter = *reinterpret_cast<IteratorImpl *>(iter_);

	if (iter.first) {
		iter.first = false;
	} else {
		++iter.iter;
	}

	if (iter.iter == std::default_sentinel) {
		return TENTRIS_I_ENDED;
	}

	static_assert(sizeof(TripleStoreIterator::value_type) == sizeof(tentris_triple)
				  && alignof(TripleStoreIterator::value_type) == alignof(tentris_triple));

	*out_triple = reinterpret_cast<tentris_triple const *>(&*iter.iter);
	return TENTRIS_I_YIELDED;
}

void tentris_triplestore_iterator_destroy(tentris_triplestore_iterator *iter) {
	reinterpret_cast<IteratorImpl *>(iter)->~IteratorImpl();
}


#ifdef TENTRIS_FFI_WITH_HYPERTRIE
hypertrie const *tentris_triplestore_get_hypertrie(tentris_triplestore const *ts_) {
	auto const &ts = *reinterpret_cast<TripleStore const *>(ts_);
	auto const &hyp = ts.hypertrie();

	using ffi_htt_t = dice::hypertrie::internal::ffi::bool_htt_t;
	using tentris_htt_t = dice::tentris::defs::htt_t;

	// Best effort detection if reinterpret cast below is safe
	static_assert(sizeof(ffi_htt_t::key_part_type) == sizeof(tentris_htt_t::key_part_type)
				  && alignof(ffi_htt_t::key_part_type) == alignof(tentris_htt_t::key_part_type)
				  && std::is_same_v<ffi_htt_t::value_type, tentris_htt_t::value_type>
				  && ffi_htt_t::taggable_key_part == tentris_htt_t::taggable_key_part
				  && std::is_same_v<typename ffi_htt_t::template set_type<int, std::allocator<std::byte>>, typename tentris_htt_t::template set_type<int, std::allocator<std::byte>>>
				  && std::is_same_v<typename ffi_htt_t::template map_type<int, int, std::allocator<std::byte>>, typename tentris_htt_t::template map_type<int, int, std::allocator<std::byte>>>
				  //&& std::is_same_v<ffi_alloc_t , tentris_alloc_t>); // TODO pretty sure they are equal but for some reason this fails, maybe compiler bug. Best effort workaround below.
				  && std::is_same_v<dice::hypertrie::internal::ffi::std_allocator_type, dice::tentris::defs::std_allocator_type>
				  && std::is_same_v<dice::hypertrie::internal::ffi::metall_allocator_type, dice::tentris::defs::metall_allocator_type>);

	auto const *hypx = reinterpret_cast<dice::hypertrie::internal::ffi::BoolHypertrie const *>(&hyp);
	return reinterpret_cast<hypertrie const *>(new dice::hypertrie::internal::ffi::AnyHypertrie{std::in_place_type<dice::hypertrie::internal::ffi::const_BoolHypertrie>, *hypx});
}
#endif//TENTRIS_FFI_WITH_HYPERTRIE
