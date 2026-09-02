#define DICE_TENTRIS_TEST_CASE_NAME "DeleteData"
#include "tests_Commons.hpp"

#include <vector>
#include "dice/sparql/parser/SPARQLParser.hpp"

namespace dice::tests::sparql {

	/* SPARQL Cast Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/delete-data) */
	TEST_CASE("SPARQL Delete Data Queries") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/delete-data/";

		run_sparql_delete_data_test_case_antlr(const_url, "delete-pre-01.ttl", "delete-data-01.ru", "delete-post-01s.ttl");
		run_sparql_delete_data_test_case_dedicated_parser(const_url, "delete-pre-01.ttl", "delete-data-01.ru", "delete-post-01s.ttl");
		// uses GRAPH
		// run_sparql_delete_data_test_case_antlr(const_url, "delete-pre-01.ttl", "delete-02.ru", "delete-post-01s.ttl");
		run_sparql_delete_data_test_case_antlr(const_url, "delete-pre-01.ttl", "delete-data-03.ru", "delete-post-01f.ttl");
		run_sparql_delete_data_test_case_dedicated_parser(const_url, "delete-pre-01.ttl", "delete-data-03.ru", "delete-post-01f.ttl");
		// uses GRAPH
		// run_sparql_delete_data_test_case_antlr(const_url, "delete-pre-01.ttl", "delete-03.ru", "delete-post-01f.ttl");
		// uses multiple graphs
		// run_sparql_delete_data_test_case_antlr(const_url, "", "delete-05.ru", "");
		// uses multiple graphs
		// run_sparql_delete_data_test_case_antlr(const_url, "", "delete-06.ru", "");

		SUBCASE("Malformed Queries") {
			char const *delete_with_var = "DELETE DATA { ?x <p> <o> }";
			char const *insert_with_var = "INSERT DATA { ?x <p> <o> }";
			char const *delete_with_bnode = "DELETE DATA { _:a <p> <o> }";
			std::vector<dice::sparql::SPARQLQuery> no_discard_dummy_1;
			dice::sparql::SPARQLQuery no_discard_dummy_2{{}};
			CHECK_THROWS_WITH(no_discard_dummy_1 = dice::sparql::parser::SPARQLParser::parse_update(delete_with_var), doctest::Contains("Parse Error: Malformed Query"));
			CHECK_THROWS_WITH(no_discard_dummy_2 = dice::sparql::parser::SPARQLParser::parse_delete_or_insert_data(delete_with_var), doctest::Contains("Parse Error: Malformed Query"));
			CHECK_THROWS_WITH(no_discard_dummy_1 = dice::sparql::parser::SPARQLParser::parse_update(insert_with_var), doctest::Contains("Parse Error: Malformed Query"));
			CHECK_THROWS_WITH(no_discard_dummy_2 = dice::sparql::parser::SPARQLParser::parse_delete_or_insert_data(insert_with_var), doctest::Contains("Parse Error: Malformed Query"));
			CHECK_THROWS_WITH(no_discard_dummy_1 = dice::sparql::parser::SPARQLParser::parse_update(delete_with_bnode), doctest::Contains("Parse Error: Malformed Query"));
			CHECK_THROWS_WITH(no_discard_dummy_2 = dice::sparql::parser::SPARQLParser::parse_delete_or_insert_data(delete_with_bnode), doctest::Contains("Parse Error: Malformed Query"));
		}
	}
}// namespace dice::tests::sparql