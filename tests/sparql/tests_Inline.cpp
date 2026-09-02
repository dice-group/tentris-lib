#define DICE_TENTRIS_TEST_CASE_NAME "Inline"
#include "tests_Commons.hpp"

namespace dice::tests::sparql {

	/* SPARQL Inline Test Queries (https://github.com/w3c/rdf-tests/tree/main/sparql/sparql11/bindings) */
	TEST_CASE("SPARQL Inline Queries") {
		char const *const_url = "https://raw.githubusercontent.com/w3c/rdf-tests/main/sparql/sparql11/bindings/";

		run_sparql_test_case(const_url, "data01.ttl", "inline01.rq", "inline01.srx", false);
		run_sparql_test_case(const_url, "data02.ttl", "inline02.rq", "inline02.srx", false);
	}

}// namespace dice::tests::sparql