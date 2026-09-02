#define DICE_TENTRIS_TEST_CASE_NAME "Bind"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL BIND Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/bind) */
	TEST_CASE("SPARQL BIND Tests") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/bind/";

		run_sparql_test_case(const_url, "data.ttl", "bind01.rq", "bind01.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind02.rq", "bind02.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind03.rq", "bind03.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind04.rq", "bind04.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind05.rq", "bind05.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind06.rq", "bind06.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind07.rq", "bind07.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind08.rq", "bind08.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind10.rq", "bind10.srx", false);
		run_sparql_test_case(const_url, "data.ttl", "bind11.rq", "bind11.srx", false);
	}

}// namespace dice::tests::sparql