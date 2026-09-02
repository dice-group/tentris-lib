#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/triplestore/TripleStore.hpp>

namespace dice::tests::triplestore {

	TEST_SUITE("tests_Construct") {

		TEST_CASE("Simple Construct") {
			using namespace dice::triplestore;
			using namespace rdf4cpp::rdf;
			using namespace rdf4cpp::rdf::query;

			TripleStore triple_store{defs::in_memory};

			triple_store.add_statement({IRI("s1"), IRI("p1"), IRI("o1")});
			triple_store.add_statement({IRI("s2"), IRI("p1"), IRI("o2")});
			triple_store.add_statement({IRI("s3"), IRI("p2"), IRI("o3")});
			triple_store.flush();

			std::string sparql_str = "CONSTRUCT { _:s1 <p1> ?o } WHERE { ?s <p1> ?o }";

			auto sparql_query = triple_store.parse_sparql_query(sparql_str);

			std::vector<sparql::detail::Entry> triples{};

			for (auto const &triple : triple_store.eval_sparql_query(*sparql_query)) {
				REQUIRE(triple.size() == 3);
				triples.push_back(triple);
			}

			REQUIRE(triples.size() == 2);
			REQUIRE((triples[0][0].as_blank_node().identifier() == "0" or triples[0][0].as_blank_node().identifier() == "1"));
			REQUIRE((triples[1][0].as_blank_node().identifier() == "0" or triples[1][0].as_blank_node().identifier() == "1"));
			REQUIRE((triples[0][2].as_iri().identifier() == "o1" or triples[0][2].as_iri().identifier() == "o2"));
			REQUIRE((triples[1][2].as_iri().identifier() == "o1" or triples[1][2].as_iri().identifier() == "o2"));
			REQUIRE((triples[0][0] != triples[1][0] and triples[1][0] != triples[1][2]));
		}

	}

}// namespace dice::tests::triplestore