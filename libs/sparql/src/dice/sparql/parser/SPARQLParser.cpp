#include "SPARQLParser.hpp"

#include <dice/sparql/parser/exception/SPARQLErrorListener.hpp>
#include <dice/sparql/parser/visitors/PrologueVisitor.hpp>
#include <dice/sparql/parser/visitors/QueryUpdateVisitor.hpp>

#include <SparqlLexer/SparqlLexer.h>
#include <SparqlParser/SparqlParser.h>

namespace dice::sparql::parser {

	SPARQLQuery SPARQLParser::parse_query(std::string const &sparql_query_str,
										  rdf4cpp::rdf::storage::node::NodeStorage const &node_storage) {
		SPARQLQuery sparql_query{std::make_shared<detail::HypertrieContext>(detail::allocator_type{})};
		// prepare antlr4 parser
		exception::SPARQLErrorListener error_listener{};
		antlr4::ANTLRInputStream input(sparql_query_str);
		dice::sparql_parser::base::SparqlLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		dice::sparql_parser::base::SparqlParser parser(&tokens);
		parser.removeErrorListeners();
		parser.addErrorListener(&error_listener);
		// check if the provided string is a QueryUnit (https://www.w3.org/TR/sparql11-query/#rQueryUnit)
		auto query_ctx = parser.query();
		if (not query_ctx)
			throw exception::parse_error{0, 0, "The provided query is not a QueryUnit"};
		// parse the prefixes
		robin_hood::unordered_map<std::string, std::string> prefixes;
		if (auto prologue_ctx = query_ctx->prologue(); prologue_ctx) {
			prefixes = std::any_cast<robin_hood::unordered_map<std::string, std::string>>(visitors::PrologueVisitor{}.visitPrologue(query_ctx->prologue()));
		}
		// parse the query
		visitors::QueryUpdateVisitor query_visitor{sparql_query, prefixes, node_storage};
		if (auto ask_ctx = query_ctx->askQuery(); ask_ctx)
			query_visitor.visitAskQuery(ask_ctx);
		else if (auto select_ctx = query_ctx->selectQuery(); select_ctx)
			query_visitor.visitSelectQuery(select_ctx);
		else if (auto construct_ctx = query_ctx->constructQuery(); construct_ctx)
			query_visitor.visitConstructQuery(construct_ctx);
		else
			// only DESCRIBE is missing
			throw exception::unsupported_query{0, 0, "Only SELECT, ASK and CONSTRUCT queries are currently supported"};
		return sparql_query;
	}

	std::vector<SPARQLQuery> SPARQLParser::parse_update(std::string const &sparql_query_str,
														rdf4cpp::rdf::storage::node::NodeStorage const &node_storage) {
		std::vector<SPARQLQuery> sparql_queries{};
		// prepare antlr4 parser
		exception::SPARQLErrorListener error_listener{};
		antlr4::ANTLRInputStream input(sparql_query_str);
		dice::sparql_parser::base::SparqlLexer lexer(&input);
		antlr4::CommonTokenStream tokens(&lexer);
		dice::sparql_parser::base::SparqlParser parser(&tokens);
		parser.removeErrorListeners();
		parser.addErrorListener(&error_listener);
		// check if the provided string is a QueryUnit (https://www.w3.org/TR/sparql11-query/#rQueryUnit)
		auto update_ctx = parser.updateCommand();
		if (not update_ctx)
			throw exception::parse_error{0, 0, "The provided query is not an UpdateUnit"};
		robin_hood::unordered_map<std::string, std::string> prefixes;
		auto tensor_context = std::make_shared<detail::HypertrieContext>(detail::allocator_type{});

		for (size_t i = 0; i < update_ctx->update().size(); i++) {
			// parse the prefixes
			// overwrites previous values if a key is encountered in multiple prologues
			for (auto &[key, value] : std::any_cast<robin_hood::unordered_map<std::string, std::string>>(visitors::PrologueVisitor{}.visitPrologue(update_ctx->prologue(i)))) {
				prefixes[key] = value;
			}
			// create query object
			SPARQLQuery sparql_query{tensor_context};
			// parse the query
			visitors::QueryUpdateVisitor query_visitor{sparql_query, prefixes, node_storage};
			query_visitor.visitUpdate(update_ctx->update(i));
			sparql_queries.push_back(std::move(sparql_query));
		}
		return sparql_queries;
	}

	SPARQLQuery SPARQLParser::parse_delete_or_insert_data(std::string const &sparql_query_str) {
		SPARQLQuery sparql_query{std::make_shared<detail::HypertrieContext>(detail::allocator_type{})};

		if (!parse_delete_or_insert_data_impl(sparql_query_str, sparql_query)) {
			throw exception::parse_error{0, 0, "The provided query is not an INSERT DATA or a DELETE DATA operation"};
		}

		return sparql_query;
	}

	/* helper functions for INSERT DATA/DELETE DATA parser */

	static bool is_alpha(char const ch) noexcept {
		return std::isalpha(ch);
	}

	static bool is_ws(char const ch) noexcept {
		return std::isspace(ch);
	}

	/**
	 * @brief reads a single word (determined by matcher) from the start of s
	 * @param s input string; will be modified to not include the word after extraction
	 * @param matcher determines the charset the word is made of
	 * @return the extracted word
	 * @todo when clang supports ranges properly: merge read_word and read_word_rev
	 */
	template<typename CharMatcher>
		requires std::is_nothrow_invocable_r_v<bool, CharMatcher, char>
	static std::string_view read_word(std::string_view &s, CharMatcher &&matcher) noexcept {
		auto const first_word_begin = std::find_if_not(s.begin(), s.end(), is_ws);
		auto const first_word_end = std::find_if_not(first_word_begin, s.end(), std::forward<CharMatcher>(matcher));

		auto word = s.substr(std::distance(s.begin(), first_word_begin), std::distance(first_word_begin, first_word_end));
		s.remove_prefix(std::distance(s.begin(), first_word_end));

		return word;
	}

	/**
	 * @brief reads a single word (determined by matcher) from the end of s
	 * @param s input string; will be modified to not include the word after extraction
	 * @param matcher determines the charset the word is made of
	 * @return the extracted word
	 */
	template<typename CharMatcher>
		requires std::is_nothrow_invocable_r_v<bool, CharMatcher, char>
	static std::string_view read_word_rev(std::string_view &s, CharMatcher &&matcher) noexcept {
		auto const first_word_rbegin = std::find_if_not(s.rbegin(), s.rend(), is_ws);
		auto const first_word_rend = std::find_if_not(first_word_rbegin, s.rend(), std::forward<CharMatcher>(matcher));

		auto word = s.substr(std::distance(first_word_rend, s.rend()), std::distance(first_word_rbegin, first_word_rend));
		s.remove_suffix(std::distance(s.rbegin(), first_word_rend));

		return word;
	}

	/**
	 * @brief extracts the prologue from an update query
	 * @param s the whole query, will be modified to not include the extracted prologue afterwards
	 * @return the extracted prologue
	 */
	static std::string_view read_prologue(std::string_view &s) noexcept {
		auto const query_body_begin = s.find_first_of('{');
		if (query_body_begin == std::string_view::npos) {
			// body begin not found, error will be handled by calling function
			return "";
		}

		auto const prologue_last_char = s.substr(0, query_body_begin).find_last_of('>');
		if (prologue_last_char == std::string_view::npos) {
			// no prologue found
			return "";
		}

		auto const prologue = s.substr(0, prologue_last_char + 1);
		s.remove_prefix(prologue_last_char + 1);

		return prologue;
	}

	/* end of helper functions */

	bool SPARQLParser::parse_delete_or_insert_data_impl(std::string const &sparql_update_str, SPARQLQuery &sparql_query) {
		std::string_view sparql_update_view = sparql_update_str;

		auto prologue = read_prologue(sparql_update_view);

		auto const first_word = read_word(sparql_update_view, is_alpha);
		auto const second_word = read_word(sparql_update_view, is_alpha);

		if (second_word != "DATA")
			return false;

		SPARQLQuery::QueryType query_type;

		if (first_word == "DELETE")
			query_type = SPARQLQuery::QueryType::DELETE_DATA;
		else if (first_word == "INSERT")
			query_type = SPARQLQuery::QueryType::INSERT_DATA;
		else
			return false;

		sparql_query.set_query_type(query_type);

		auto const third_word = read_word(sparql_update_view, [](char const ch) noexcept { return ch == '{'; });

		if (third_word != "{") {
			// missing (or too many) '{' after '(DELETE|INSERT) DATA'
			std::ostringstream err;
			err << "syntax error: expected '{' after " << (query_type == SPARQLQuery::QueryType::DELETE_DATA ? "DELETE DATA" : "INSERT DATA");
			throw exception::malformed_query{0, 0, err.str()};
		}

		auto const last_word = read_word_rev(sparql_update_view, [](char const ch) noexcept { return ch == '}'; });

		if (last_word != "}") {
			// closing brace is missing from query
			throw exception::malformed_query{0, 0, "syntax error: expected '}' at end of query"};
		}

		rdf4cpp::rdf::parser::IStreamQuadIterator::prefix_storage_type prefixes;
		using namespace rdf4cpp::rdf::parser;
		{// parse only prologue using antlr
			parser::exception::SPARQLErrorListener error_listener{};
			antlr4::ANTLRInputStream input{prologue};
			dice::sparql_parser::base::SparqlLexer lexer{&input};
			antlr4::CommonTokenStream tokens{&lexer};
			dice::sparql_parser::base::SparqlParser parser{&tokens};
			parser.removeErrorListeners();
			parser.addErrorListener(&error_listener);
			{// visit prologue and store prefixes
				parser::visitors::PrologueVisitor p_visitor{};
				for (auto const &[key, value] : std::any_cast<robin_hood::unordered_map<std::string, std::string>>(p_visitor.visitPrologue(parser.prologue()))) {
					prefixes[key] = value;
				}
			}
		}

		std::vector<rdf4cpp::rdf::query::QuadPattern> quad_template{};
		{// try to parse all triples between '{' and '}' with rdf4cpp and then store them in 'entries'
			std::istringstream iss{std::string{sparql_update_view}};
			for (IStreamQuadIterator qit{iss, ParsingFlag::NoParsePrefix, prefixes}; qit != IStreamQuadIterator{}; ++qit) {
				if (qit->has_value()) {
					auto bnode_check = [&query_type](rdf4cpp::rdf::Node node) {
						return node.is_blank_node() and query_type == SPARQLQuery::QueryType::DELETE_DATA;
					};
					if (std::any_of((*qit)->begin(), (*qit)->end(), bnode_check)) {
						throw exception::malformed_query{qit->get_unexpected().value().line, qit->get_unexpected().value().col, "BLANK NODE in DELETE DATA"};
					}
					quad_template.push_back(**qit);
				} else {
					std::ostringstream oss;
					oss << qit->error();
					throw exception::malformed_query{qit->error().line, qit->error().col, oss.str()};
				}
			}
		}
		if (sparql_query.query_type() == SPARQLQuery::QueryType::DELETE_DATA)
			sparql_query.set_delete_template(std::move(quad_template));
		else
			sparql_query.set_insert_template(std::move(quad_template));
		return true;
	}

}// namespace dice::sparql::parser