#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/triplestore/TripleStore.hpp>

TEST_CASE("Dangling Evaluation Context") {
	using namespace dice::triplestore;
	using namespace rdf4cpp::rdf;

	TripleStore ts{defs::in_memory};
	ts.add_statement(Statement{IRI{"https://triples.org#subj"}, IRI{"https://triples.org#pred"}, IRI{"https://triples.org#obj"}});

	auto query = ts.parse_sparql_query("SELECT (COUNT(*) as ?ntriples) WHERE { ?s ?p ?o . }");
	auto solutions = ts.eval_sparql_query(*query);

	for (auto const &sol : solutions) {
		CHECK_EQ(sol.size(), 1);
		CHECK_EQ(sol.key()[0], Literal::make_typed_from_value<datatypes::xsd::Integer>(1));
	}
}
