#define DICE_TENTRIS_TEST_CASE_NAME "Exists"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL EXISTS Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/exists) */
	TEST_CASE("SPARQL EXISTS Queries") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/exists/";

		run_sparql_test_case(const_url, "exists01.ttl", "exists01.rq", "exists01.srx", false);
		run_sparql_test_case(const_url, "exists01.ttl", "exists02.rq", "exists02.srx", false);
		run_sparql_test_case(const_url, "exists01.ttl", "exists04.rq", "exists04.srx", false);
		run_sparql_test_case(const_url, "exists01.ttl", "exists05.rq", "exists05.srx", false);
	}

}// namespace dice::tests::sparql