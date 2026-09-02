#define DICE_TENTRIS_TEST_CASE_NAME "ExpressionOperations"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL Expression Operations Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql10/expr-ops) */
	TEST_CASE("SPARQL Expression Operations") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql10/expr-ops/";

		run_sparql_test_case(const_url, "data.ttl", "query-ge-1.rq", "result-ge-1.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "query-le-1.rq", "result-le-1.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "query-plus-1.rq", "result-plus-1.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "query-mul-1.rq", "result-mul-1.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "query-minus-1.rq", "result-minus-1.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "query-unminus-1.rq", "result-unminus-1.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "query-unplus-1.rq", "result-unplus-1.srx", false);
	}
}