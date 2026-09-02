#define DICE_TENTRIS_TEST_CASE_NAME "Algebra"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL FILTER Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql10/algebra) */
	TEST_CASE("SPARQL Algebra Tests (OPTIONAL, JOIN combination, FILTER)") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql10/algebra/";

		/* join-combo */
		run_sparql_test_case(const_url, "join-combo-graph-2.ttl", "join-combo-1.rq", "join-combo-1.srx", false);
		// contains GRAPH
		//run_sparql_test_case(const_url, "join-combo-graph-2.ttl", "join-combo-2.rq", "join-combo-2.srx", false);
		/* nested-opt */
		// not well-designed
		// run_sparql_test_case(const_url, "two-nested-opt.ttl", "two-nested-opt.rq", "two-nested-opt.srx", false);
		// not well-designed (weakly-well designed)
		//run_sparql_test_case(const_url, "two-nested-opt.ttl", "two-nested-opt-alt.rq", "two-nested-opt-alt.srx", false);
		/* opt-filter */
		run_sparql_test_case(const_url, "opt-filter-1.ttl", "opt-filter-1.rq", "opt-filter-1.srx", false);
		run_sparql_test_case(const_url, "opt-filter-2.ttl", "opt-filter-2.rq", "opt-filter-2.srx", false);
		run_sparql_test_case(const_url, "opt-filter-3.ttl", "opt-filter-3.rq", "opt-filter-3.srx", false);
		/* filter */
		run_sparql_test_case(const_url, "data-2.ttl", "filter-placement-1.rq", "filter-placement-1.srx", false);
		run_sparql_test_case(const_url, "data-2.ttl", "filter-placement-2.rq", "filter-placement-2.srx", false);
		run_sparql_test_case(const_url, "data-2.ttl", "filter-placement-3.rq", "filter-placement-3.srx", false);
		run_sparql_test_case(const_url, "data-1.ttl", "filter-nested-1.rq", "filter-nested-1.srx", false);
		run_sparql_test_case(const_url, "data-1.ttl", "filter-nested-2.rq", "filter-nested-2.srx", false);
		/* join-scope */
		// not well-designed
		// run_sparql_test_case(const_url, "var-scope-join-1.ttl", "var-scope-join-1.rq", "var-scope-join-1.srx", false);
	}

}// namespace dice::tests::sparql