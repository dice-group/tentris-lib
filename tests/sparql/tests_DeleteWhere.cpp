#define DICE_TENTRIS_TEST_CASE_NAME "DeleteWhere"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL Cast Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/delete-where) */
	TEST_CASE("SPARQL Delete Where Queries") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/delete-where/";

		run_sparql_update_test_case(const_url, "delete-pre-01.ttl", "delete-where-01.ru", "delete-post-01s.ttl");
		run_sparql_update_test_case(const_url, "delete-pre-01.ttl", "delete-where-03.ru", "delete-post-01f.ttl");
	}
}// namespace dice::tests::sparql