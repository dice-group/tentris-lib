#include "dice/sparql/parser/exception/Exceptions.hpp"

#include <sstream>

namespace dice::sparql::parser::exception {

	static std::string make_error_msg(size_t line, size_t col, std::string const &parser_msg) {
		std::ostringstream error_msg;
		error_msg << "Parse Error: " << parser_msg << ". At line: " << line << " and position: " << col;
		return error_msg.str();
	}

	parse_error::parse_error(size_t line, size_t col, std::string const &parser_msg) : std::runtime_error{make_error_msg(line, col, parser_msg)},
																					   line_{line},
																					   col_{col},
																					   parser_msg_{parser_msg} {
	}

	std::string const &parse_error::parser_msg() const noexcept {
		return parser_msg_;
	}

	size_t parse_error::line() const noexcept {
		return line_;
	}

	size_t parse_error::col() const noexcept {
		return col_;
	}

	unsupported_query::unsupported_query(size_t line, size_t col, std::string const &operation) : parse_error{line, col, "Unsupported Feature or Operation: " + operation} {
	}

	std::string const &unsupported_query::unsupported_operation() const noexcept {
		return unsupported_operation_;
	}

	malformed_query::malformed_query(size_t line, size_t col, std::string const &parser_msg) : parse_error{line, col, "Malformed Query: " + parser_msg} {
	}

} // dice::sparql::parser::exception