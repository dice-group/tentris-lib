#define DICE_TENTRIS_TEST_CASE_NAME "SolutionSequence"
#include "tests_Commons.hpp"


namespace dice::tests::sparql {
	/* SPARQL Subquery Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql10/solution-seq) */
	/* The results of the test cases hardcoded; not in xml format */
	TEST_CASE("SPARQL Solution Sequence") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql10/solution-seq/";

		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-01.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-02.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-03.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-04.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-10.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-11.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-12.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-13.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-20.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-21.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-22.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-23.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "data.ttl", "slice-24.rq", false, false);
	}

}// namespace dice::tests::sparql