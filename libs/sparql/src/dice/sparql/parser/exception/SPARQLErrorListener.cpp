#include "dice/sparql/parser/exception/SPARQLErrorListener.hpp"

namespace dice::sparql::parser::exception {

	void SPARQLErrorListener::syntaxError([[maybe_unused]] antlr4::Recognizer *recognizer,
										  [[maybe_unused]] antlr4::Token *offendingSymbol,
										  size_t line,
										  size_t charPositionInLine,
										  std::string const &msg,
										  [[maybe_unused]] std::exception_ptr e) {
		throw parse_error{line, charPositionInLine, msg};
	}

}// namespace dice::sparql::parser::exception