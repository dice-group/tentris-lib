#define DICE_TENTRIS_TEST_CASE_NAME "Construct"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL Construct Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/construct,
	 * 								  https://github.com/w3c/rdf-tests/tree/main/sparql/sparql10/construct) *
	 */
	TEST_CASE("SPARQL Construct Queries") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/construct/";
		char const *const_url2 = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql10/construct/";

		run_sparql_construct_test_case(const_url2, "data-ident.ttl", "query-ident.rq", "result-ident.ttl", false);
		run_sparql_construct_test_case(const_url2, "data-opt.ttl", "query-construct-optional.rq", "result-construct-optional.ttl", false);
		run_sparql_construct_test_case(const_url2, "data-ident.ttl", "query-subgraph.rq", "result-subgraph.ttl", false);
		// run_sparql_construct_test_case(const_url2, "data-reif.ttl", "query-reif-1.rq", "result-reif.ttl", false);
		run_sparql_construct_test_case(const_url2, "data-reif.ttl", "query-reif-2.rq", "result-reif.ttl", false);
		// shorthand version (sparql11)
		run_sparql_construct_test_case(const_url, "data.ttl", "constructwhere01.rq", "constructwhere01result.ttl", false);
		run_sparql_construct_test_case(const_url, "data.ttl", "constructwhere02.rq", "constructwhere02result.ttl", false);
		run_sparql_construct_test_case(const_url, "data.ttl", "constructwhere03.rq", "constructwhere03result.ttl", false);
		// contains from clause
		// run_sparql_construct_test_case(const_url, "data.ttl", "constructwhere04.rq", "constructwhere04result.ttl", false);
		run_sparql_test_case_parse_exception(const_url, "data.ttl", "constructwhere05.rq", "Parse Error", false);
		run_sparql_test_case_parse_exception(const_url, "data.ttl", "constructwhere06.rq", "Parse Error", false);
	}
}// namespace dice::tests::sparql
