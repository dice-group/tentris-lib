#ifndef TENTRIS_QUERY_SPARQL_PROLOGUEVISITOR_HPP
#define TENTRIS_QUERY_SPARQL_PROLOGUEVISITOR_HPP

#include <SparqlParser/SparqlParserBaseVisitor.h>

#include <robin_hood.h>


namespace dice::sparql::parser::visitors {

	using namespace dice::sparql_parser;

	class PrologueVisitor : public base::SparqlParserBaseVisitor {
	private:
		robin_hood::unordered_map<std::string, std::string> prefixes_;

	public:
		std::any visitPrologue(base::SparqlParser::PrologueContext *) override;
		std::any visitBaseDecl(base::SparqlParser::BaseDeclContext *) override;
		std::any visitPrefixDecl(base::SparqlParser::PrefixDeclContext *) override;
	};

}// namespace dice::sparql::parser::visitors

#endif//TENTRIS_QUERY_SPARQL_PROLOGUEVISITOR_HPP
