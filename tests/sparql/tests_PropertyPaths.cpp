#define DICE_TENTRIS_TEST_CASE_NAME "ProperyPaths"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL Functions Test Queries (https://github.com/w3c/rdf-tests/blob/main/sparql/sparql11/project-expression/) */
	TEST_CASE("SPARQL Queries with Property Paths") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/property-path/";

		run_sparql_test_case(const_url, "pp01.ttl", "pp01.rq", "pp01.srx", false);
		// contains star
		//run_sparql_test_case(const_url, "pp01.ttl", "pp02.rq", "pp02.srx", false);
		run_sparql_test_case(const_url, "pp03.ttl", "pp03.rq", "pp03.srx", false);
		// contains graphs
		//run_sparql_test_case(const_url, "pp06.ttl", "pp06.rq", "pp06.srx", false);
		// contains graphs
		//run_sparql_test_case(const_url, "pp07.ttl", "pp06.rq", "pp07.srx", false);
		run_sparql_test_case(const_url, "pp08.ttl", "pp08.rq", "pp08.srx", false);
		run_sparql_test_case(const_url, "pp09.ttl", "pp09.rq", "pp09.srx", false);
		// contains negated set
		//run_sparql_test_case(const_url, "pp09.ttl", "pp10.rq", "pp10.srx", false);
		run_sparql_test_case(const_url, "pp11.ttl", "pp11.rq", "pp11.srx", false);
		// contains plus
		//run_sparql_test_case(const_url, "pp11.ttl", "pp12.rq", "pp12.srx", false);
		// contains star
		//run_sparql_test_case(const_url, "pp14.ttl", "pp14.rq", "pp14.srx", false);
		// contains star
		//run_sparql_test_case(const_url, "pp14.ttl", "pp14.rq", "pp14.srx", false);
		// contains plus
		// run_sparql_test_case(const_url, "data-diamond.ttl", "path-2-2.rq", "diamond-2.srx", false);
		// contains plus
		// run_sparql_test_case(const_url, "data-diamond-tail.ttl", "path-2-2.rq", "diamond-tail-2.srx", false);
		// contains ?
		// run_sparql_test_case(const_url, "data-diamond-loop.ttl", "path-3-3.rq", "diamond-loop-5a.srx", false);
		run_sparql_test_case(const_url, "path-p1.ttl", "path-p1.rq", "path-p1.srx", false);
		run_sparql_test_case(const_url, "path-p1.ttl", "path-p2.rq", "path-p2.srx", false);
		run_sparql_test_case(const_url, "path-p3.ttl", "path-p3.rq", "path-p3.srx", false);
	}

}// namespace dice::tests::sparql