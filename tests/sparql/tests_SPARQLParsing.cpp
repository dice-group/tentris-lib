#define DICE_TENTRIS_TEST_CASE_NAME "SPARQLParsing"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests_Commons.hpp"

#include <dice/sparql/parser/SPARQLParser.hpp>


namespace dice::tests::sparql {

	TEST_SUITE("GroupGraphPattern to OperandDependencyGraph") {

		TEST_CASE("Single Triple Pattern") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE {?s ?p ?o}");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 1);
			CHECK(odg.isc_operands().size() == 1);
			CHECK(odg.operand_var_ids(0).size() == 3);
		}

		TEST_CASE("Multiple Triple Patterns") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE {?s ?p ?o . ?s ?p1 ?o1 . ?s ?p2 ?o2 .}");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 3);
			CHECK(odg.isc_operands().size() == 3);
			CHECK(odg.operand_var_ids(0)[0] == odg.operand_var_ids(1)[0]);
			CHECK(odg.operand_var_ids(0)[0] == odg.operand_var_ids(2)[0]);
		}

		TEST_CASE("TriplesNodePath as Object") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s ?p [ ?p1 ?o1 ] }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 2);
			CHECK(odg.isc_operands().size() == 2);
		}

		TEST_CASE("Single UNION") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { {?s ?p ?o} UNION {?s1 ?p1 ?o1} }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 2);
			CHECK(odg.union_components().size() == 2);
		}

		TEST_CASE("Multiple (x2) UNIONs") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { {?s ?p ?o} UNION {?s1 ?p1 ?o1} {?s3 ?p3 ?o3} UNION {?s4 ?p4 ?o4} }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 8);
			CHECK(odg.union_components().size() == 4);
		}

		TEST_CASE("PathSequence (/)") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s <p1>/<p2> ?o }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 2);
			CHECK(odg.isc_operands().size() == 2);
		}

		TEST_CASE("PathAlternative (|)") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s <p1>|<p2>|<p3> ?o }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 3);
			CHECK(odg.union_components().size() == 3);
		}

		TEST_CASE("Single Filter") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s ?p ?o FILTER(?o>5) }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 2);
			CHECK(odg.isc_operands().size() == 2);
			CHECK(odg.operand_var_ids(0)[2] == odg.operand_var_ids(1)[0]);
		}

		TEST_CASE("Filter ConditionalAnd") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s ?p ?o FILTER(?o>5 && ?o<10) }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 3);
			CHECK(odg.isc_operands().size() == 3);
		}

		TEST_CASE("Filter Variable Scope (not in-scope)") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { { ?s ?p ?o } { FILTER(?o>5) } }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 2);
			CHECK(odg.isc_operands().size() == 2);
			CHECK(odg.operand_var_ids(0)[2] != odg.operand_var_ids(1)[0]);
		}

		TEST_CASE("Filter Variable Scope (in-scope)") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s ?p ?o { FILTER(?o>5) } }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 2);
			CHECK(odg.isc_operands().size() == 2);
			CHECK(odg.operand_var_ids(0)[2] != odg.operand_var_ids(1)[0]);
		}

		TEST_CASE("Inverted Property, Variable Position") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s ?p ?o . ?s ^<p1> ?t . }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 2);
			CHECK(odg.isc_operands().size() == 2);
			CHECK(odg.operand_var_ids(0)[0] == odg.operand_var_ids(1)[1]);
		}

		TEST_CASE("Multiple OPTIONALs") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s <p> <o> . ?s ?p ?o OPTIONAL { ?p <p1> ?o1 } OPTIONAL { ?p <p2> ?o2 } }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 4);
		}

		TEST_CASE("Multiple OPTIONALs (x3)") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s <p> <o> . ?s ?p ?o OPTIONAL { ?p <p1> ?o1 } OPTIONAL { ?p <p2> ?o2 } OPTIONAL { ?p <p3> ?o3 } }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 5);
		}

		TEST_CASE("UNION within OPTIONAL") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s <p> <o> . OPTIONAL { { ?s <p1> <o1> } UNION { ?s <p2> <o2> } } }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 3);
		}

		TEST_CASE("UNION within OPTIONAL and multiple OPTIONALs") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { ?s <p> <o> . OPTIONAL {?s <p3> <o3> } OPTIONAL { { ?s <p1> <o1> } UNION { ?s <p2> <o2> } } }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 4);
		}

		TEST_CASE("Distribution of OPTIONAL over UNION") {
			auto parsed_query = dice::sparql::parser::SPARQLParser::parse_query("SELECT * WHERE { { ?s <p> <o> } UNION { ?s <p4> <o4> }  OPTIONAL {?s <p3> <o3> } OPTIONAL { { ?s <p1> <o1> } UNION { ?s <p2> <o2> } } }");
			auto const &odg = parsed_query.operand_dependency_graph();
			CHECK(odg.size() == 8);
		}
	}
}// namespace dice::tests::sparql
