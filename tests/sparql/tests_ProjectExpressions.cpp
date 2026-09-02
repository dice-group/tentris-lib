#define DICE_TENTRIS_TEST_CASE_NAME "ProjectExpressions"
#include "tests_Commons.hpp"


namespace dice::tests::sparql {

	/* SPARQL Functions Test Queries (https://github.com/w3c/rdf-tests/blob/main/sparql/sparql11/project-expression/) */
	TEST_CASE("SPARQL Queries with Expressions in Projections") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/project-expression/";

		run_sparql_test_case(const_url, "projexp01.ttl", "projexp01.rq", "projexp01.srx", false);
		run_sparql_test_case(const_url, "projexp02.ttl", "projexp02.rq", "projexp02.srx", false);
		run_sparql_test_case(const_url, "projexp03.ttl", "projexp03.rq", "projexp03.srx", false);
		run_sparql_test_case(const_url, "projexp04.ttl", "projexp04.rq", "projexp04.srx", false);
		run_sparql_test_case(const_url, "projexp05.ttl", "projexp05.rq", "projexp05.srx", false);
		run_sparql_test_case(const_url, "projexp06.ttl", "projexp06.rq", "projexp06.srx", false);
		run_sparql_test_case(const_url, "projexp07.ttl", "projexp07.rq", "projexp07.srx", false);
	}

}// namespace dice::tests::sparql