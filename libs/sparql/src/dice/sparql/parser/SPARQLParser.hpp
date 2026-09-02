#ifndef TENTRIS_QUERY_SPARQL_SPARQLPARSER_HPP
#define TENTRIS_QUERY_SPARQL_SPARQLPARSER_HPP

#include <dice/sparql/SPARQLQuery.hpp>

namespace dice::sparql::parser {

	class SPARQLParser {
	public:
		/**
		 * @brief parses the given SPARQL query
		 * @param sparql_query_str the query
		 * @param eval_ctx evaluation context
		 * @return parsed SPARQL query
		 * @throws exception::parser_error if the provided SPARQL query string could not be parsed
		 * @throws exception::unsupported_query if the provided SPARQL query is not SELECT or ASK
		 * @throws exception::malformed_query if the provided SPARQL query is malformed in a non-syntactic way
		 */
		[[nodiscard]] static SPARQLQuery parse_query(std::string const &sparql_query_str,
													 rdf4cpp::rdf::storage::node::NodeStorage const &node_storage = rdf4cpp::rdf::storage::node::NodeStorage::default_instance());

		/**
	 	 * @brief parses the given SPARQL update query
	 	 * @param sparql_query_str the query
	 	 * @param eval_ctx evaluation context
	 	 * @return vector of parsed SPARQL queries (one SPARQL query for each update operation)
	 	 */
		[[nodiscard]] static std::vector<SPARQLQuery> parse_update(std::string const &sparql_query_str,
																   rdf4cpp::rdf::storage::node::NodeStorage const &node_storage = rdf4cpp::rdf::storage::node::NodeStorage::default_instance());

		/**
	  	 * @brief Dedicated parser function for single (large) SPARQL INSERT DATA or DELETE DATA update operations
	 	 * @param sparql_query_str the query
	 	 * @param eval_ctx evaluation context
	 	 * @return parsed SPARQL query
	 	 * @throws exception::parser_error if the provided SPARQL query string could not be parsed
	 	 * @throws exception::malformed_query if the provided SPARQL query is malformed in a non-syntactic way
	 	 */
		[[nodiscard]] static SPARQLQuery parse_delete_or_insert_data(std::string const &sparql_query_str);

	private:
		static bool parse_delete_or_insert_data_impl(std::string const &sparql_query_str, SPARQLQuery &sparql_query);
	};

}// namespace dice::sparql::parser

#endif// TENTRIS_QUERY_SPARQL_SPARQLPARSER_HPP
