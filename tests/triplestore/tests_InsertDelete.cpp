#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/triplestore/TripleStore.hpp>

namespace dice::tests::triplestore {

	TEST_SUITE("tests_InsertDelete") {

		TEST_CASE("Insert") {
			using namespace dice::triplestore;
			using namespace rdf4cpp::rdf;
			using namespace rdf4cpp::rdf::query;

			TripleStore triple_store{defs::in_memory};

			triple_store.add_statement({IRI("s1"), IRI("p1"), IRI("o1")});
			triple_store.add_statement({IRI("s2"), IRI("p1"), IRI("o2")});
			triple_store.add_statement({IRI("s3"), IRI("p2"), IRI("o3")});
			triple_store.flush();

			std::string sparql_str = "INSERT { <s4> ?p <o1> } WHERE { ?s ?p <o1> }";

			auto sparql_update_queries = triple_store.parse_sparql_update(sparql_str);

			triple_store.eval_sparql_update(sparql_update_queries[0]);

			REQUIRE(triple_store.size() == 4);
			REQUIRE(triple_store.hypertrie().get_cards({2})[0] == 3); // ensure that we use the existing id for o1

			sparql_str = "INSERT { _:s <p1> ?o } WHERE { ?s <p1> ?o }";

			sparql_update_queries = triple_store.parse_sparql_update(sparql_str);
			triple_store.eval_sparql_update(sparql_update_queries[0]);

			REQUIRE(triple_store.size() == 6);
			REQUIRE(triple_store.hypertrie().get_cards({0})[0] == 6); // ensure that we have new blank nodes for each binding
			REQUIRE(triple_store.hypertrie().get_cards({1})[0] == 2);
			REQUIRE(triple_store.hypertrie().get_cards({2})[0] == 3);
		}

		TEST_CASE("DELETE DATA") {
			using namespace dice::triplestore;
			using namespace rdf4cpp::rdf;
			using namespace rdf4cpp::rdf::query;

			TripleStore triple_store{defs::in_memory};

			triple_store.add_statement({IRI("s1"), IRI("p1"), IRI("o1")});
			triple_store.add_statement({IRI("s2"), IRI("p1"), IRI("o2")});
			triple_store.add_statement({IRI("s3"), IRI("p2"), IRI("o3")});
			triple_store.flush();

			std::string sparql_str = "DELETE DATA { <s3> <p2> <o3> }";

			auto sparql_update_queries = triple_store.parse_sparql_update(sparql_str);

			triple_store.eval_sparql_update(sparql_update_queries[0]);

			REQUIRE(triple_store.size() == 2);
		}

		TEST_CASE("INSERT DATA") {
			using namespace dice::triplestore;
			using namespace rdf4cpp::rdf;
			using namespace rdf4cpp::rdf::query;

			TripleStore triple_store{defs::in_memory};

			triple_store.add_statement({IRI("s1"), IRI("p1"), IRI("o1")});
			triple_store.add_statement({IRI("s2"), IRI("p1"), IRI("o2")});
			triple_store.add_statement({IRI("s3"), IRI("p2"), IRI("o3")});
			triple_store.flush();

			std::string sparql_str = "INSERT DATA { <s3> <p2> <o4> . <s4> <p3> <o4> . } ";

			auto sparql_update_queries = triple_store.parse_sparql_update(sparql_str);
			triple_store.eval_sparql_update(sparql_update_queries[0]);

			REQUIRE(triple_store.size() == 5);
			REQUIRE(triple_store.contains({IRI("s3"), IRI("p2"), IRI("o4")}));
			REQUIRE(triple_store.contains({IRI("s4"), IRI("p3"), IRI("o4")}));
		}

		TEST_CASE("DELETE and INSERT") {
			using namespace dice::triplestore;
			using namespace rdf4cpp::rdf;
			using namespace rdf4cpp::rdf::query;

			TripleStore triple_store{defs::in_memory};

			triple_store.add_statement({IRI("s1"), IRI("p1"), IRI("o1")});
			triple_store.add_statement({IRI("s2"), IRI("p1"), IRI("o2")});
			triple_store.add_statement({IRI("s3"), IRI("p2"), IRI("o3")});
			triple_store.flush();

			std::string sparql_str = "DELETE { ?s <p1> ?o } INSERT { ?s <p3> ?o } WHERE { ?s <p1> ?o }";

			auto sparql_update_queries = triple_store.parse_sparql_update(sparql_str);
			triple_store.eval_sparql_update(sparql_update_queries[0]);

			REQUIRE(triple_store.size() == 3);
			REQUIRE_FALSE(triple_store.contains({IRI("s1"), IRI("p1"), IRI("o1")}));
			REQUIRE_FALSE(triple_store.contains({IRI("s2"), IRI("p1"), IRI("o2")}));
			REQUIRE(triple_store.contains({IRI("s1"), IRI("p3"), IRI("o1")}));
			REQUIRE(triple_store.contains({IRI("s2"), IRI("p3"), IRI("o2")}));

		}

	}

}// namespace dice::tests::triplestore