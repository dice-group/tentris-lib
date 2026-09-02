#define DICE_TENTRIS_TEST_CASE_NAME "Aggregates"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL Aggregate Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/aggregates) */
	TEST_CASE("SPARQL Aggregate Queries") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/aggregates/";

		run_sparql_test_case(const_url, "agg01.ttl", "agg01.rq", "agg01.srx", false);
		run_sparql_test_case(const_url, "agg01.ttl", "agg02.rq", "agg02.srx", false);
		run_sparql_test_case(const_url, "agg01.ttl", "agg03.rq", "agg03.srx", false);
		run_sparql_test_case(const_url, "agg01.ttl", "agg04.rq", "agg04.srx", false);
		run_sparql_test_case(const_url, "agg01.ttl", "agg05.rq", "agg05.srx", false);
		run_sparql_test_case(const_url, "agg01.ttl", "agg06.rq", "agg06.srx", false);
		run_sparql_test_case(const_url, "agg01.ttl", "agg07.rq", "agg07.srx", false);
		run_sparql_test_case_parse_exception(const_url, "agg08.ttl", "agg08.rq", "Parse Error: Malformed Query: Non-aggregated variable ?O2 is not part of the group key. At line: 3 and position: 0", false);
		run_sparql_test_case(const_url, "agg08.ttl", "agg08b.rq", "agg08b.srx", false);
		run_sparql_test_case_parse_exception(const_url, "agg08.ttl", "agg09.rq", "Parse Error: Malformed Query: Non-aggregated variable ?P is not part of the group key. At line: 3 and position: 0", false);
		run_sparql_test_case_parse_exception(const_url, "agg08.ttl", "agg10.rq", "Parse Error: Malformed Query: Non-aggregated variable ?P is not part of the group key. At line: 3 and position: 0", false);
		run_sparql_test_case_parse_exception(const_url, "agg08.ttl", "agg11.rq", "Parse Error: Malformed Query: Non-aggregated variable ?O2 is not part of the group key. At line: 3 and position: 0", false);
		run_sparql_test_case_parse_exception(const_url, "agg08.ttl", "agg12.rq", "Parse Error: Malformed Query: Non-aggregated variable ?O1 is not part of the group key. At line: 3 and position: 0", false);
		run_sparql_test_case(const_url, "agg-numeric.ttl", "agg-max-01.rq", "agg-max-01.srx", false);
		run_sparql_test_case(const_url, "agg-numeric.ttl", "agg-max-02.rq", "agg-max-02.srx", false);
		run_sparql_test_case(const_url, "agg-numeric.ttl", "agg-min-01.rq", "agg-min-01.srx", false);
		run_sparql_test_case(const_url, "agg-numeric.ttl", "agg-min-02.rq", "agg-min-02.srx", false);
		run_sparql_test_case(const_url, "agg-numeric.ttl", "agg-avg-01.rq", "agg-avg-01.srx", false);
		// run_sparql_test_case(const_url, "agg-numeric2.ttl", "agg-avg-02.rq", "agg-avg-02.srx", false); // This test is currently failing because of rdf4cpp; will be fixed when #175 is merged
		run_sparql_test_case(const_url, "agg-numeric.ttl", "agg-sum-01.rq", "agg-sum-01.srx", false);
		run_sparql_test_case(const_url, "agg-numeric2.ttl", "agg-sum-02.rq", "agg-sum-02.srx", false);
		run_sparql_test_case(const_url, "agg-numeric.ttl", "agg-sample-01.rq", "agg-sample-01.srx", false);
		run_sparql_test_case(const_url, "empty.ttl", "agg-empty-group-max-1.rq", "agg-empty-group-max-1.srx", false);
		run_sparql_test_case(const_url, "empty.ttl", "agg-empty-group-max-2.rq", "agg-empty-group-max-2.srx", false);
		run_sparql_test_case_hardcoded_results(const_url, "empty.ttl", "agg-empty-group-count-1.rq", false, false);
		run_sparql_test_case_hardcoded_results(const_url, "empty.ttl", "agg-empty-group-count-2.rq", false, false);
		run_sparql_test_case(const_url, "agg-err-01.ttl", "agg-err-01.rq", "agg-err-01.srx", false);
		// missing features (if, coalesce)
		// run_sparql_test_case(const_url, "agg-err-02.ttl", "agg-err-02.rq", "agg-err-02.srx", false);
	}

}// namespace dice::tests::sparql