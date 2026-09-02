#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/triplestore/TripleStore.hpp>

namespace dice::tests::triplestore {

	TEST_SUITE("tests_NodeStorage") {

		std::vector<std::map<rdf4cpp::rdf::query::Variable, node_wrapper::NodeWrapper>> eval_query(const std::shared_ptr<const sparql::SPARQLQuery>& sparql_query,
																								   dice::triplestore::TripleStore &triple_store) {
			std::vector<std::map<rdf4cpp::rdf::query::Variable, node_wrapper::NodeWrapper>> results{};
			for (auto const &entry : triple_store.eval_sparql_query(*sparql_query)) {
				for (size_t i = 0; i < entry.value(); i++) {
					std::map<rdf4cpp::rdf::query::Variable, node_wrapper::NodeWrapper> result{};
					for (size_t j = 0; j < entry.key().size(); j++) {
						if (entry.key()[j].null())
							continue;
						result[sparql_query->projected_variables()[j]] = entry.key()[j];
					}
					results.push_back(result);
				}
			}
			return results;
		}

		TEST_CASE("SPARQL uses triple store's node storage") {
			using namespace dice::triplestore;
			using namespace rdf4cpp::rdf;
			using namespace rdf4cpp::rdf::query;

			TripleStore triple_store{defs::in_memory};

			triple_store.add_statement({IRI("s1"), IRI("p1"), IRI("o1")});
			triple_store.add_statement({IRI("s2"), IRI("p1"), IRI("o2")});
			triple_store.add_statement({IRI("s3"), IRI("p2"), IRI("o3")});
			triple_store.add_statement({IRI("s4"), IRI("p4"), IRI("o1")});
			triple_store.add_statement({IRI("s4"), IRI("p4"), IRI("o2")});
			triple_store.add_statement({IRI("s5"), IRI("p3"), Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Double>("5")});
			triple_store.flush();

			std::string sparql_str = "SELECT * WHERE { ?s <p2> ?o }";
			auto sparql_query = triple_store.parse_sparql_query(sparql_str);
			auto results = eval_query(sparql_query, triple_store);
			REQUIRE(results[0][Variable("s")] == IRI("s3"));
			REQUIRE(results[0][Variable("o")] == IRI("o3"));

			sparql_str = "SELECT ?s ?t1 WHERE { ?s <p4> <o1> . ?s <p4> ?t1 . FILTER (?t1 != <o1>) }";
			sparql_query = triple_store.parse_sparql_query(sparql_str);
			results = eval_query(sparql_query, triple_store);
			REQUIRE(results[0][Variable("s")] == IRI("s4"));
			REQUIRE(results[0][Variable("t1")] == IRI("o2"));

			sparql_str = "SELECT ?s ?t1 WHERE { ?s <p4> <o1> . ?s <p4> ?t1 . FILTER (?t1 = <o1>) }";
			sparql_query = triple_store.parse_sparql_query(sparql_str);
			results = eval_query(sparql_query, triple_store);
			REQUIRE(results[0][Variable("s")] == IRI("s4"));
			REQUIRE(results[0][Variable("t1")] == IRI("o1"));

			sparql_str = "SELECT ?s WHERE { ?s <p3> ?l . FILTER (?l = 5) }";
			sparql_query = triple_store.parse_sparql_query(sparql_str);
			results = eval_query(sparql_query, triple_store);
			REQUIRE(results[0][Variable("s")] == IRI("s5"));

			sparql_str = "SELECT ?s WHERE { ?s <p3> ?l . FILTER (?l < 6) }";
			sparql_query = triple_store.parse_sparql_query(sparql_str);
			results = eval_query(sparql_query, triple_store);
			REQUIRE(results[0][Variable("s")] == IRI("s5"));

			sparql_str = "SELECT ?s WHERE { ?s <p3> ?l . FILTER (?l > 4.0) }";
			sparql_query = triple_store.parse_sparql_query(sparql_str);
			results = eval_query(sparql_query, triple_store);
			REQUIRE(results[0][Variable("s")] == IRI("s5"));

			sparql_str = "SELECT ?s WHERE { ?s <p3> ?l . FILTER (?l < 5.0) }";
			sparql_query = triple_store.parse_sparql_query(sparql_str);
			results = eval_query(sparql_query, triple_store);
			REQUIRE(results.empty());

			sparql_str = "SELECT ?s WHERE { ?s <p3> ?l . FILTER (?l > 5.000) }";
			sparql_query = triple_store.parse_sparql_query(sparql_str);
			results = eval_query(sparql_query, triple_store);
			REQUIRE(results.empty());

			sparql_str = "SELECT ?s WHERE { ?s <p3> ?l . FILTER (?l != 5.0) }";
			sparql_query = triple_store.parse_sparql_query(sparql_str);
			results = eval_query(sparql_query, triple_store);
			REQUIRE(results.empty());
		}

		TEST_CASE("Literal escape") {
			using namespace dice::triplestore;
			using namespace rdf4cpp::rdf;
			using namespace rdf4cpp::rdf::query;

			TripleStore triple_store{defs::in_memory};

			auto u1 = triple_store.parse_sparql_update("INSERT DATA { <http://abc.com#s1> <http://abc.com#p> \"New\\n            Line.\" . }");
			auto u2 = triple_store.parse_sparql_insert_or_delete_data("INSERT DATA { <http://abc.com#s2> <http://abc.com#p> \"New\\n            Line.\" . }");
			triple_store.eval_sparql_update(u1[0]);
			triple_store.eval_sparql_update(u2);

			auto q = triple_store.parse_sparql_query("CONSTRUCT WHERE { ?s ?p ?o }");

			for (auto const &triple : triple_store.eval_sparql_query(*q)) {
				CHECK_EQ(triple.size(), 3);
				auto obj = std::string{triple.key()[2]};
				CAPTURE(obj);
				CHECK(obj.find('\n') == std::string::npos);
			}
		}
	}

}// namespace dice::tests::triplestore