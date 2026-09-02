#ifndef TENTRIS_QUERY_SPARQL_EXCEPTIONS_HPP
#define TENTRIS_QUERY_SPARQL_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace dice::sparql::parser::exception {

	struct parse_error : std::runtime_error {
	private:
		size_t line_;
		size_t col_;
		std::string parser_msg_;

	public:
		parse_error(size_t line, size_t col, std::string const &parser_msg);

		[[nodiscard]] std::string const &parser_msg() const noexcept;
		[[nodiscard]] size_t line() const noexcept;
		[[nodiscard]] size_t col() const noexcept;
	};

	struct unsupported_query : parse_error {
	private:
		std::string unsupported_operation_;

	public:
		unsupported_query(size_t line, size_t col, std::string const &operation);

		/**
		 * @return the operation that triggered this exception
		 */
		[[nodiscard]] std::string const &unsupported_operation() const noexcept;
	};

	struct malformed_query : parse_error {
		malformed_query(size_t line, size_t col, std::string const &parser_msg);
	};

} // namespace dice::sparql::parser::exception

#endif //TENTRIS_QUERY_SPARQL_EXCEPTIONS_HPP
