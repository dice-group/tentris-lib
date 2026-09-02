#define DICE_TENTRIS_TEST_CASE_NAME "Cast"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL Cast Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/cast) */
	TEST_CASE("SPARQL Cast Queries") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/cast/";

		run_sparql_test_case(const_url, "data.ttl", "cast-double.rq", "cast-double.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "cast-float.rq", "cast-float.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "cast-int.rq", "cast-int.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "cast-bool.rq", "cast-bool.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "cast-decimal.rq", "cast-decimal.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "cast-string.rq", "cast-string.srx", false);
	}
}// namespace dice::tests::sparql