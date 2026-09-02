#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "dice/triplestore/TripleStore.hpp"
#include <random>

void insert_random_data(dice::triplestore::TripleStore &ts) {
	static constexpr char const *iri_base = "http://testing.com#";

	std::default_random_engine rng{std::random_device{}()};

	for (size_t ix = 0; ix < 10'000; ++ix) {
		std::string const iri1 = iri_base + std::to_string(rng());
		std::string const iri2 = iri_base + std::to_string(rng());
		std::string const iri3 = iri_base + std::to_string(rng());

		ts.add_statement(rdf4cpp::rdf::Statement{rdf4cpp::rdf::IRI{iri1},
												 rdf4cpp::rdf::IRI{iri2},
												 rdf4cpp::rdf::IRI{iri3}});
	}

	ts.flush();
}

/**
 * Observed behaviour differences using different compilers:
 *
 * - clang-14: Had some problems in the past, but didn't recheck if it works now.
 * - clang-15: Works. Quirk: The generator does not seem to start before the first element after construction, meaning it _can_ throw on construction.
 * - clang-16: Crashes with SIGABRT "free(): invalid pointer"
 *
 * - gcc-13: Works. Generator seems to start before the first element on construction and therefore _cannot_ throw in the constructor.
 */
TEST_CASE("timeout") {
	using namespace dice::triplestore;

	TripleStore ts{defs::in_memory};
	insert_random_data(ts);

	auto query = ts.parse_sparql_query("SELECT DISTINCT * WHERE {\n"
									   "?s ?p ?o . \n"
									   "_:b1 ?p2 _:b2 . \n"
									   "}");

	try {
		auto gen = ts.eval_sparql_query(*query, std::chrono::steady_clock::now() + std::chrono::seconds{1});

		for (auto const &solution : gen) {
			(void) solution;
		}

		CHECK_FALSE("Expected timeout");
	} catch (dice::query::query_timeout const &) {
		// expecting timeout
	}
}
