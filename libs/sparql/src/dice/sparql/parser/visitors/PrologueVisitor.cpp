#include "dice/sparql/parser/visitors/PrologueVisitor.hpp"
#include "dice/sparql/parser/exception/Exceptions.hpp"

namespace dice::sparql::parser::visitors {

	std::any PrologueVisitor::visitPrologue(base::SparqlParser::PrologueContext *ctx) {
		prefixes_.clear();
		for (auto pref_ctx : ctx->prefixDecl())
			visitPrefixDecl(pref_ctx);
		for ([[maybe_unused]] auto base_ctx : ctx->baseDecl())
			throw exception::unsupported_query{ctx->getStart()->getLine(),
											   ctx->getStart()->getCharPositionInLine(),
											   "Base Declarations not supported yet."};
		return prefixes_;
	}

	std::any PrologueVisitor::visitBaseDecl([[maybe_unused]] base::SparqlParser::BaseDeclContext *ctx) {
		return nullptr;
	}

	std::any PrologueVisitor::visitPrefixDecl(base::SparqlParser::PrefixDeclContext *ctx) {
		std::string prefix{};
		if (ctx->PNAME_NS())
			prefix = ctx->PNAME_NS()->getText();
		auto ns = ctx->IRIREF()->getText();
		prefixes_[prefix.substr(0, prefix.size() - 1)] = ns.substr(1, ns.size() - 2);
		return nullptr;
	}


}// namespace dice::sparql2tensor::parser::visitors