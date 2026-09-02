#ifndef TENTRIS_QUERY_SPARQL_SPARQLERRORLISTENER_HPP
#define TENTRIS_QUERY_SPARQL_SPARQLERRORLISTENER_HPP

#include "dice/sparql/parser/exception/Exceptions.hpp"

#include <BaseErrorListener.h>

#include <robin_hood.h>


namespace dice::sparql::parser::exception {

	/**
	 * @brief A custom error listener for ANTLR. Throws exception instead of printing to stderr.
	 */
	class SPARQLErrorListener : public antlr4::BaseErrorListener {

		void syntaxError(antlr4::Recognizer *recognizer,
						 antlr4::Token *offendingSymbol,
						 size_t line,
						 size_t charPositionInLine,
						 const std::string &msg,
						 std::exception_ptr e) override;

	};

}// namespace dice::sparql2tensor::parser::exception

#endif//TENTRIS_QUERY_SPARQL_SPARQLERRORLISTENER_HPP
