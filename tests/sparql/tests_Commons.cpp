#define DOCTEST_CONFIG_IMPLEMENT
#include "tests_Commons.hpp"

#include <dice/metall-node-storage/MetallNodeStorageBackend.hpp>
#include <dice/tentris/hypertrie-template-instantiation.hpp>
#include <dice/sparql.hpp>

#include <rdf4cpp/rdf.hpp>

#include <iostream>
#include <sstream>

#include <curl/curl.h>
#include <pugixml.hpp>

int main(int argc, char **argv) {
	using namespace dice::metall_node_storage;
	using namespace rdf4cpp::rdf::storage::node;

	NodeStorage::set_default_instance(NodeStorage::new_instance<MetallNodeStorageBackend>(defs::in_memory));
	return doctest::Context{argc, argv}.run();
}

namespace dice::tests::sparql {
	using ::dice::node_wrapper::NodeWrapper;
	using namespace ::dice::sparql;
	using rdf4cpp::rdf::IRI;
	using rdf4cpp::rdf::Literal;
	using rdf4cpp::rdf::query::Variable;

	static dice::tentris::defs::allocator_type get_allocator() {
		return {dice::tentris::defs::std_allocator_type{}};
	}

	static dice::tentris::defs::HypertrieContext_ptr get_hypertrie_context() {
		static dice::tentris::defs::HypertrieContext context{get_allocator()};
		return {&context};
	}

	static detail::BoolHypertrie load_data(std::string const &rdf_data) {
		using namespace rdf4cpp::rdf::parser;

		detail::BoolHypertrie rdf_tensor{3, get_hypertrie_context()};
		if (rdf_data.empty()) {
			// workaround for bug in rdf4cpp
			return rdf_tensor;
		}

		std::istringstream iss{rdf_data};
		for (IStreamQuadIterator quad_iter{iss}; quad_iter != IStreamQuadIterator{}; ++quad_iter) {
			REQUIRE_MESSAGE(quad_iter->has_value(), "Error while parsing rdf data: " << quad_iter->error());

			auto const &triple = quad_iter->value();
			rdf_tensor.set({triple.subject(), triple.predicate(), triple.object()}, true);
		}
		return rdf_tensor;
	}

	static std::vector<std::map<Variable, NodeWrapper>> eval_sparql_query(std::string const &query_str,
																		  detail::const_BoolHypertrie const &tensor) {
		// create context
		detail::EvaluationContext eval_ctx(tensor);
		// create sparql query
		auto sparql_query = dice::sparql::parser::SPARQLParser::parse_query(query_str);
		// evaluate sparql query
		std::vector<std::map<rdf4cpp::rdf::query::Variable, node_wrapper::NodeWrapper>> actual_results{};
		auto &raw_query = sparql_query.raw_query();
		for (auto const &entry : detail::QueryEvaluation::evaluate(raw_query, eval_ctx)) {
			for (size_t i = 0; i < entry.value(); i++) {
				std::map<rdf4cpp::rdf::query::Variable, node_wrapper::NodeWrapper> result{};
				for (size_t j = 0; j < entry.key().size(); j++) {
					if (entry.key()[j].null())
						continue;
					result[sparql_query.projected_variables()[j]] = entry.key()[j];
				}
				actual_results.push_back(result);
			}
		}
		return actual_results;
	}

	static void eval_sparql_update_query(std::string const &query_str, detail::BoolHypertrie &tensor) {
		// create context
		detail::EvaluationContext eval_ctx(tensor);
		// parse sparql update query
		auto sparql_queries = dice::sparql::parser::SPARQLParser::parse_update(query_str);
		// evaluate all update operations
		for (auto &sparql_query : sparql_queries) {
			// update eval_ctx after each operation
			detail::EvaluationContext update_eval_ctx(tensor);
			auto &raw_query = sparql_query.raw_query();
			std::vector<hypertrie::internal::raw::SingleEntry<3, detail::htt_t>> to_remove{};
			std::vector<hypertrie::internal::raw::SingleEntry<3, detail::htt_t>> to_insert{};
			for (auto const &entry : detail::QueryEvaluation::evaluate(raw_query, update_eval_ctx)) {
				// iterate over delete template (if available)
				for (auto const &quad_pattern : sparql_query.delete_template()) {
					hypertrie::internal::raw::SingleEntry<3, detail::htt_t> delete_entry{};
					size_t i = 0;
					bool skip = false;
					// skip graph position
					for (auto iter = quad_pattern.begin() + 1; iter != quad_pattern.end(); iter++) {
						auto term = *iter;
						if (term.is_variable()) {
							delete_entry.key()[i] = entry[sparql_query.quad_template_variable_position(term.as_variable())];
						} else {
							delete_entry.key()[i] = term;
						}
						if (delete_entry.key()[i].null()) {
							skip = true;
							break;
						}
						i++;
					}
					if (skip)
						continue;
					to_remove.push_back(delete_entry);
				}
				// iterate over insert template (if available)
				for (auto const &quad_pattern : sparql_query.insert_template()) {
					hypertrie::internal::raw::SingleEntry<3, detail::htt_t> insert_entry{};
					size_t i = 0;
					bool skip = false;
					// skip graph position
					for (auto iter = quad_pattern.begin() + 1; iter != quad_pattern.end(); iter++) {
						auto term = *iter;
						if (term.is_variable()) {
							insert_entry.key()[i] = entry[sparql_query.quad_template_variable_position(term.as_variable())];
						} else {
							insert_entry.key()[i] = term;
						}
						if (insert_entry.key()[i].null()) {
							skip = true;
							break;
						}
						i++;
					}
					if (skip)
						continue;
					to_insert.push_back(insert_entry);
				}
			}
			// bulk removal
			{
				tentris::defs::HypertrieAsyncBulkRemover bulk_remover{tensor};
				for (auto const &to_rem : to_remove) {
					bulk_remover.add(to_rem.key());
				}
			}
			// bulk insertion
			{
				tentris::defs::HypertrieAsyncBulkInserter bulk_inserter{tensor};
				for (auto const &to_ins : to_insert) {
					bulk_inserter.add(to_ins);
				}
			}
		}
	}

	static detail::BoolHypertrie eval_sparql_construct_query(std::string const &query_str, detail::const_BoolHypertrie &tensor) {
		// create context
		detail::EvaluationContext eval_ctx(tensor);
		// create sparql query
		auto sparql_query = dice::sparql::parser::SPARQLParser::parse_query(query_str);
		auto &raw_query = sparql_query.raw_query();
		detail::BoolHypertrie actual_results_rdf_tensor{3, get_hypertrie_context()};
		detail::HypertrieSyncBulkInserter bulk_inserter{actual_results_rdf_tensor, 1000000};
		size_t bnode_id = 0;
		for (auto const &entry : detail::QueryEvaluation::evaluate(raw_query, eval_ctx)) {
			// The blank node labels are scoped to the template for each solution
			std::map<rdf4cpp::rdf::query::Variable, size_t> bnode_to_id{};
			for (auto const &triple_pattern : sparql_query.construct_template()) {
				hypertrie::internal::raw::SingleEntry<3, detail::htt_t> insert_entry{};
				size_t i = 0;
				bool skip = false;
				for (auto const &term : triple_pattern) {
					if (term.is_variable()) {
						if (term.as_variable().is_anonymous()) {
							auto bnode = term.as_variable();
							if (not bnode_to_id.contains(bnode))
								bnode_to_id[bnode] = bnode_id++;
							insert_entry.key()[i] = rdf4cpp::rdf::BlankNode::make(std::to_string(bnode_to_id[bnode]));
						} else {
							insert_entry.key()[i] = entry[sparql_query.quad_template_variable_position(term.as_variable())];
						}
					} else {
						insert_entry.key()[i] = term;
					}
					if (insert_entry.key()[i].null()) {
						skip = true;
						break;
					}
					i++;
				}
				if (skip)
					continue;
				bulk_inserter.add(insert_entry);
			}
		}
		bulk_inserter.flush();
		std::cout << "Actual Results (CONSTRUCT)" << std::endl;
		for (auto const &entry : actual_results_rdf_tensor) {
			std::cout << to_string(entry) << std::endl;
		}
		return actual_results_rdf_tensor;
	}

	static bool compare_results(std::vector<std::map<rdf4cpp::rdf::query::Variable, NodeWrapper>> actual_results,
								std::vector<std::map<rdf4cpp::rdf::query::Variable, NodeWrapper>> expected_results,
								bool sort = true) {
		if (sort) {
			std::sort(expected_results.begin(), expected_results.end());
			std::sort(actual_results.begin(), actual_results.end());
		}
		std::cout << "Actual Results" << std::endl;
		for (auto const &result : actual_results) {
			for (auto const &[var, binding] : result) {
				std::cout << var << ":" << binding << " ";
			}
			std::cout << std::endl;
		}
		std::cout << "Expected Results" << std::endl;
		for (auto const &result : expected_results) {
			for (auto const &[var, binding] : result) {
				std::cout << var << ":" << binding << " ";
			}
			std::cout << std::endl;
		}
		return (actual_results == expected_results);
	}

	static std::vector<std::map<Variable, NodeWrapper>> parse_sparql_result_file(std::string const &sparql_results) {
		pugi::xml_document query_results;
		pugi::xml_parse_result parsing_result = query_results.load_string(sparql_results.c_str());
		assert(parsing_result);
		std::vector<std::map<Variable, NodeWrapper>> expected_results{};
		if (auto bool_res = query_results.child("sparql").child("boolean"); bool_res) {
			if (strcmp(bool_res.first_child().value(), "true") == 0)
				expected_results.emplace_back();
		} else {
			for (const auto &result : query_results.child("sparql").child("results").children()) {
				std::map<Variable, NodeWrapper> single_result{};
				for (const auto &binding : result.children()) {
					auto var = Variable(binding.attribute("name").value());
					rdf4cpp::rdf::Node term{};
					if (auto uri = binding.child("uri"); uri) {
						term = IRI(uri.first_child().value());
					} else if (auto literal = binding.child("literal"); literal) {
						auto lexical_form = literal.first_child().value();
						if (auto datatype = literal.attribute("datatype"); datatype) {
							term = Literal::make_typed(lexical_form, IRI(datatype.value()));
						} else if (auto lang_tag = literal.attribute("xml:lang")) {
							term = Literal::make_lang_tagged(lexical_form, lang_tag.value());
						} else {
							term = Literal::make_simple(lexical_form);
						}
					} else if (auto bnode = binding.child("bnode"); bnode) {
						term = rdf4cpp::rdf::BlankNode(bnode.first_child().value());
					} else {
						assert(false);
					}
					single_result[var] = term;
				}
				expected_results.push_back(std::move(single_result));
			}
		}
		return expected_results;
	}

	// from: https://stackoverflow.com/a/9786295
	static size_t curl_write_callback_function(void *contents, size_t size, size_t nmemb, void *userp) {
		(static_cast<std::string *>(userp))->append(static_cast<char *>(contents), size * nmemb);
		return size * nmemb;
	}

	static std::string read_file_from_url(std::string const &url) {
		CURL *curl;
		curl = curl_easy_init();
		std::string curl_result;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback_function);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curl_result);
		auto res = curl_easy_perform(curl);
		assert(res == CURLE_OK);
		curl_easy_cleanup(curl);
		return curl_result;
	}

	static std::vector<std::map<Variable, NodeWrapper>> get_hardcoded_results(std::string const &test_case) {
		if (test_case == "slice-01.rq" or test_case == "slice-20.rq")
			return {
					{{Variable("v"), Literal::make_typed("1", IRI("http://www.w3.org/2001/XMLSchema#integer"))}}};
		else if (test_case == "slice-02.rq" or test_case == "slice-11.rq")
			return {
					{{Variable("v"), Literal::make_typed("1", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("1", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("1.5", IRI("http://www.w3.org/2001/XMLSchema#decimal"))}},
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("4", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},

			};
		else if (test_case == "slice-03.rq" or test_case == "slice-12.rq" or test_case == "slice-22.rq")
			return {};
		else if (test_case == "slice-04.rq")
			return {
					{{Variable("v"), Literal::make_typed("1", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("1.5", IRI("http://www.w3.org/2001/XMLSchema#decimal"))}},
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("4", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},

			};
		else if (test_case == "slice-10.rq")
			return {
					{{Variable("v"), Literal::make_typed("1", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("1.5", IRI("http://www.w3.org/2001/XMLSchema#decimal"))}},
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("4", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
			};
		else if (test_case == "slice-13.rq")
			return {
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("4", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},

			};
		else if (test_case == "slice-21.rq")
			return {
					{{Variable("v"), Literal::make_typed("1", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("1.5", IRI("http://www.w3.org/2001/XMLSchema#decimal"))}}};
		else if (test_case == "slice-23.rq")
			return {
					{{Variable("v"), Literal::make_typed("1.5", IRI("http://www.w3.org/2001/XMLSchema#decimal"))}},
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
			};
		else if (test_case == "slice-24.rq")
			return {
					{{Variable("v"), Literal::make_typed("2", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("3", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
					{{Variable("v"), Literal::make_typed("4", IRI("http://www.w3.org/2001/XMLSchema#integer"))}},
			};
		else if (test_case == "agg-empty-group-count-1.rq")
			return {};
		else if (test_case == "agg-empty-group-count-2.rq")
			return {
					{{Variable("C"), Literal::make_typed("0", IRI("http://www.w3.org/2001/XMLSchema#integer"))}}};
		else if (test_case == "bound1.rq")
			return {
					{{Variable("a"), IRI("http://example.org/ns#c2")},
					 {Variable("c"), IRI("http://example.org/ns#f")}},
					{{Variable("a"), IRI("http://example.org/ns#a2")},
					 {Variable("c"), IRI("http://example.org/ns#c2")}}
			};
		return {};
	}

	void run_sparql_test_case_parse_exception(char const *url, char const *data, char const *query, char const *exception, bool static_data) {
		SUBCASE(query) {
			auto const data_url = std::string{url} + data;
			auto const query_url = std::string{url} + query;
			auto const rdf_data = static_data ? std::string{data} : read_file_from_url(data_url);
			auto const rdf_tensor = load_data(rdf_data);
			auto const sparql_str = read_file_from_url(query_url);
			CHECK_THROWS_WITH(eval_sparql_query(sparql_str, rdf_tensor), doctest::Contains(exception));
		}
	}

	void run_sparql_test_case_hardcoded_results(char const *url, char const *data, char const *query, bool static_data, bool sort) {
		SUBCASE(query) {
			auto const data_url = std::string{url} + data;
			auto const query_url = std::string{url} + query;
			auto const rdf_data = static_data ? std::string{data} : read_file_from_url(data_url);
			auto const rdf_tensor = load_data(rdf_data);
			auto const sparql_str = read_file_from_url(query_url);
			auto const actual_results = eval_sparql_query(sparql_str, rdf_tensor);
			CHECK(compare_results(actual_results, get_hardcoded_results(query), sort));
		}
	}

	void run_sparql_test_case(char const *url, char const *data, char const *query, char const *result, bool static_data) {
		SUBCASE(query) {
			auto const data_url = std::string{url} + data;
			auto const query_url = std::string{url} + query;
			auto const result_url = std::string{url} + result;
			auto const rdf_data = static_data ? std::string{data} : read_file_from_url(data_url);
			auto const rdf_tensor = load_data(rdf_data);
			auto const sparql_str = read_file_from_url(query_url);
			auto const expected_results = parse_sparql_result_file(read_file_from_url(result_url));
			auto const actual_results = eval_sparql_query(sparql_str, rdf_tensor);
			CHECK(compare_results(actual_results, expected_results));
		}
	}

	void run_sparql_construct_test_case(char const *url, char const *data, char const *query, char const *result, bool static_data) {
		SUBCASE(query) {
			const std::string data_url = std::string{url} + data;
			const std::string query_url = std::string{url} + query;
			const std::string result_url = std::string{url} + result;
			const std::string rdf_data = static_data ? std::string{data} : read_file_from_url(data_url);
			auto rdf_tensor = load_data(rdf_data);
			const std::string result_rdf_data = read_file_from_url(result_url);
			auto result_rdf_tensor = load_data(result_rdf_data);
			auto sparql_str = read_file_from_url(query_url);
			auto actual_results = eval_sparql_construct_query(sparql_str, rdf_tensor);
			CHECK(actual_results.size() == result_rdf_tensor.size());
		}
	}

	void run_sparql_update_test_case_parse_exception(char const *url, char const *data, char const *query, char const *exception, bool static_data) {
		SUBCASE(query) {
			const std::string data_url = std::string{url} + data;
			const std::string query_url = std::string{url} + query;
			const std::string rdf_data = static_data ? std::string{data} : read_file_from_url(data_url);
			auto rdf_tensor = load_data(rdf_data);
			auto sparql_str = read_file_from_url(query_url);
			CHECK_THROWS_WITH(eval_sparql_update_query(sparql_str, rdf_tensor), doctest::Contains(exception));
		}
	}

	void run_sparql_update_test_case(char const *url, char const *data, char const *query, char const *result) {
		SUBCASE(query) {
			const std::string data_url = std::string{url} + data;
			const std::string query_url = std::string{url} + query;
			const std::string result_url = std::string{url} + result;
			const std::string rdf_data = read_file_from_url(data_url);
			auto rdf_tensor = load_data(rdf_data);
			const std::string result_rdf_data = read_file_from_url(result_url);
			auto result_rdf_tensor = load_data(result_rdf_data);
			auto sparql_str = read_file_from_url(query_url);
			eval_sparql_update_query(sparql_str, rdf_tensor);
			CHECK(rdf_tensor == result_rdf_tensor);
		}
	}

	void run_sparql_delete_data_test_case_antlr(char const *url, char const *data, char const *query, char const *result) {
		SUBCASE(query) {
			const std::string rdf_data = read_file_from_url(std::string{url} + data);
			const std::string result_data = read_file_from_url(std::string{url} + result);
			auto rdf_tensor = load_data(rdf_data);
			auto result_tensor = load_data(result_data);
			auto query_str = read_file_from_url(std::string{url} + query);
			auto sparql_queries = dice::sparql::parser::SPARQLParser::parse_update(query_str);
			CHECK(sparql_queries.size() == 1);
			{
				tentris::defs::HypertrieAsyncBulkRemover bulk_remover{rdf_tensor};
				for (auto const &quad : sparql_queries[0].delete_template()) {
					bulk_remover.add(tentris::defs::HTKey{{quad.subject(), quad.predicate(), quad.object()}});
				}
			}
			CHECK(rdf_tensor == result_tensor);
		}
	}

	void run_sparql_delete_data_test_case_dedicated_parser(char const *url, char const *data, char const *query, char const *result) {
		SUBCASE(query) {
			const std::string rdf_data = read_file_from_url(std::string{url} + data);
			const std::string result_data = read_file_from_url(std::string{url} + result);
			auto rdf_tensor = load_data(rdf_data);
			auto result_tensor = load_data(result_data);
			auto query_str = read_file_from_url(std::string{url} + query);
			auto sparql_query = dice::sparql::parser::SPARQLParser::parse_delete_or_insert_data(query_str);
			{
				tentris::defs::HypertrieAsyncBulkRemover bulk_remover{rdf_tensor};
				for (auto const &quad : sparql_query.delete_template()) {
					bulk_remover.add(tentris::defs::HTKey{{quad.subject(), quad.predicate(), quad.object()}});
				}
			}
			CHECK(rdf_tensor == result_tensor);
		}
	}

} // namespace dice::tests::sparql
