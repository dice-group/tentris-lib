#define DICE_TENTRIS_TEST_CASE_NAME "DeleteInsert"
#include "tests_Commons.hpp"

#include "dice/sparql/parser/SPARQLParser.hpp"


namespace dice::tests::sparql {

	/* SPARQL Cast Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/delete-insert) */
	TEST_CASE("SPARQL Delete Insert Queries") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/delete-insert/";

		run_sparql_update_test_case(const_url, "delete-insert-pre-01.ttl", "delete-insert-01.ru", "delete-insert-post-01.ttl");
		run_sparql_update_test_case(const_url, "delete-insert-pre-01.ttl", "delete-insert-01b.ru", "delete-insert-post-01b.ttl");
		run_sparql_update_test_case(const_url, "delete-insert-pre-01.ttl", "delete-insert-01c.ru", "delete-insert-post-01b.ttl");
		run_sparql_update_test_case(const_url, "delete-insert-pre-01.ttl", "delete-insert-02.ru", "delete-insert-post-02.ttl");
		// 04 uses GRAPH keyword
		// GENERATE_SPARQL_UPDATE_TEST_CASE(const_url, "delete-insert-pre-01.ttl", "delete-insert-04.ru", "delete-insert-post-02.ttl");
		run_sparql_update_test_case_parse_exception(const_url, "delete-insert-pre-01.ttl", "delete-insert-03.ru", "Parse Error: Malformed Query", false);
		run_sparql_update_test_case_parse_exception(const_url, "delete-insert-pre-01.ttl", "delete-insert-03b.ru", "Parse Error: Malformed Query", false);
		run_sparql_update_test_case_parse_exception(const_url, "delete-insert-pre-01.ttl", "delete-insert-05.ru", "Parse Error: Malformed Query", false);
		run_sparql_update_test_case_parse_exception(const_url, "delete-insert-pre-01.ttl", "delete-insert-07.ru", "Parse Error: Malformed Query", false);
		run_sparql_update_test_case_parse_exception(const_url, "delete-insert-pre-01.ttl", "delete-insert-07b.ru", "Parse Error: Malformed Query", false);
		run_sparql_update_test_case_parse_exception(const_url, "delete-insert-pre-01.ttl", "delete-insert-08.ru", "Parse Error: Malformed Query", false);
		run_sparql_update_test_case_parse_exception(const_url, "delete-insert-pre-01.ttl", "delete-insert-09.ru", "Parse Error: Malformed Query", false);
		run_sparql_update_test_case(const_url, "delete-insert-pre-01.ttl", "delete-insert-04b.ru", "delete-insert-post-02.ttl");
		run_sparql_update_test_case(const_url, "delete-insert-pre-01.ttl", "delete-insert-05b.ru", "delete-insert-post-05.ttl");
		run_sparql_update_test_case(const_url, "delete-insert-pre-06.ttl", "delete-insert-05b.ru", "delete-insert-pre-06.ttl");
		run_sparql_update_test_case(const_url, "delete-insert-halloween-problem-pre.ttl", "delete-insert-halloween-problem.ru", "delete-insert-halloween-problem-post.ttl");

		// ensures that the parsing works even if an update op does not have its own prologue
		SUBCASE("Update op without prologue") {
			char const *update_query_str = "PREFIX     : <http://example.org/> \n"
										   "PREFIX foaf: <http://xmlns.com/foaf/0.1/> \n"
										   "DELETE { ?a foaf:knows ?b . } WHERE { ?a foaf:knows ?b }\n"
										   ";\n"
										   "INSERT { ?b foaf:knows ?a . } WHERE { ?a foaf:knows ?b }";
			auto res = dice::sparql::parser::SPARQLParser::parse_update(update_query_str);
		}

	}
}// namespace dice::tests::sparql