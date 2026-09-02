#define DICE_TENTRIS_TEST_CASE_NAME "Functions"
#include "tests_Commons.hpp"


namespace dice::tests::sparql {

	/* SPARQL Functions Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/functions) */
	TEST_CASE("SPARQL Function Queries") {
		char const *const_url_1 = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/functions/";
		char const *const_url_2 = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql10/bound/";

		run_sparql_test_case(const_url_1, "data.ttl", "strdt01.rq", "strdt01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "strdt02.rq", "strdt02.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "strdt03.rq", "strdt03-rdf11.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "strlang01.rq", "strlang01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "strlang02.rq", "strlang02.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "strlang03.rq", "strlang03-rdf11.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "isnumeric01.rq", "isnumeric01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "abs01.rq", "abs01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "ceil01.rq", "ceil01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "floor01.rq", "floor01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "round01.rq", "round01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "concat01.rq", "concat01.srx", false);
		run_sparql_test_case(const_url_1, "data2.ttl", "concat02.rq", "concat02.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "substring01.rq", "substring01.srx", false);
		run_sparql_test_case(const_url_1, "data5.ttl", "substring01.rq", "substring01-non-bmp.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "substring02.rq", "substring02.srx", false);
		run_sparql_test_case(const_url_1, "data5.ttl", "substring02.rq", "substring02-non-bmp.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "length01.rq", "length01.srx", false);
		run_sparql_test_case(const_url_1, "data5.ttl", "length01.rq", "length01-non-bmp.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "ucase01.rq", "ucase01.srx", false);
		run_sparql_test_case(const_url_1, "data5.ttl", "ucase01.rq", "ucase01-non-bmp.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "lcase01.rq", "lcase01.srx", false);
		run_sparql_test_case(const_url_1, "data5.ttl", "lcase01.rq", "lcase01-non-bmp.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "encode01.rq", "encode01.srx", false);
		run_sparql_test_case(const_url_1, "data5.ttl", "encode01.rq", "encode01-non-bmp.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "contains01.rq", "contains01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "starts01.rq", "starts01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "ends01.rq", "ends01.srx", false);
		// plus-1-corrected and plus-2-corrected return correct results, but there is a mismatch in the blank_node label, which causes the equality to fail
		// run_sparql_test_case(const_url_1, "data-builtin-3.ttl", "plus-1-corrected.rq", "plus-1.srx", false);
		// run_sparql_test_case(const_url_1, "data-builtin-3.ttl", "plus-2-corrected.rq", "plus-2.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "md5-01.rq", "md5-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "md5-02.rq", "md5-02.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "sha1-01.rq", "sha1-01.srx", false);
		run_sparql_test_case(const_url_1, "hash-unicode.ttl", "sha1-02.rq", "sha1-02.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "sha256-01.rq", "sha256-01.srx", false);
		run_sparql_test_case(const_url_1, "hash-unicode.ttl", "sha256-02.rq", "sha256-02.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "sha512-01.rq", "sha512-01.srx", false);
		run_sparql_test_case(const_url_1, "hash-unicode.ttl", "sha512-02.rq", "sha512-02.srx", false);
		run_sparql_test_case(const_url_1, "hash-unicode.ttl", "sha512-02.rq", "sha512-02.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "hours-01.rq", "hours-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "minutes-01.rq", "minutes-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "seconds-01.rq", "seconds-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "year-01.rq", "year-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "month-01.rq", "month-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "day-01.rq", "day-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "timezone-01.rq", "timezone-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "tz-01.rq", "tz-01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "now01.rq", "now01.srx", false);
		// bnode with argument not supported
		//run_sparql_test_case(const_url_1, "data.ttl", "bnode01.rq", "bnode01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "in01.rq", "in01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "in02.rq", "in02.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "notin01.rq", "notin01.srx", false);
		run_sparql_test_case(const_url_1, "data.ttl", "notin02.rq", "notin02.srx", false);
		 run_sparql_test_case(const_url_1, "data.ttl", "rand01.rq", "rand01.srx", false);
		// bnode2 returns correct results; there is a mismatch in the blank_node label of the actual_results and the expected_results, which causes the equality to fail
		// run_sparql_test_case(const_url_1, "data.ttl", "bnode02.rq", "bnode02.srx", false);
		// iri01 query uses BASE in the prologue; currently not supported
		//run_sparql_test_case(const_url_1, "data.ttl", "iri01.rq", "iri01.srx", false);
		// if01 query uses BASE in the prologue; currently not supported
		//run_sparql_test_case(const_url_1, "data2.ttl", "if01.rq", "if01.srx", false);
		run_sparql_test_case(const_url_1, "data2.ttl", "if02.rq", "if02.srx", false);
		run_sparql_test_case(const_url_1, "data-coalesce.ttl", "coalesce01.rq", "coalesce01.srx", false);
		run_sparql_test_case(const_url_1, "data2.ttl", "strbefore01.rq", "strbefore01a.srx", false);
		run_sparql_test_case(const_url_1, "data4.ttl", "strbefore02.rq", "strbefore02.srx", false);
		run_sparql_test_case(const_url_1, "data2.ttl", "strafter01.rq", "strafter01a.srx", false);
		run_sparql_test_case(const_url_1, "data4.ttl", "strafter02.rq", "strafter02.srx", false);
		run_sparql_test_case(const_url_1, "data3.ttl", "replace01.rq", "replace01.srx", false);
		run_sparql_test_case(const_url_1, "data3.ttl", "replace02.rq", "replace02.srx", false);
		run_sparql_test_case(const_url_1, "data3.ttl", "replace03.rq", "replace03.srx", false);
		run_sparql_test_case(const_url_1, "data-empty.nt", "uuid01.rq", "uuid01.srx", false);
		run_sparql_test_case(const_url_1, "data-empty.nt", "uuid02.rq", "uuid02.srx", false);
		run_sparql_test_case(const_url_1, "data-empty.nt", "struuid01.rq", "struuid01.srx", false);
		run_sparql_test_case_hardcoded_results(const_url_2, "data.ttl", "bound1.rq", false, true);
	}

}// namespace dice::tests::sparql