#ifndef TENTRIS_TESTS_TESTS_COMMONS_HPP
#define TENTRIS_TESTS_TESTS_COMMONS_HPP

#include <doctest/doctest.h>

namespace dice::tests::sparql {
	void run_sparql_test_case_parse_exception(char const *url, char const *data, char const *query, char const *exception, bool static_data);
	void run_sparql_test_case_hardcoded_results(char const *url, char const *data, char const *query, bool static_data, bool sort);
	void run_sparql_test_case(char const *url, char const *data, char const * query, char const *result, bool static_data);

	void run_sparql_construct_test_case(char const *url, char const *data, char const *query, char const *result, bool static_data);

	void run_sparql_update_test_case_parse_exception(char const *url, char const *data, char const *query, char const *exception, bool static_data);
	void run_sparql_update_test_case(char const *url, char const *data, char const *query, char const *result);
	void run_sparql_delete_data_test_case_antlr(char const *url, char const *data, char const *query, char const *result);
	void run_sparql_delete_data_test_case_dedicated_parser(char const *url, char const *data, char const *query, char const *result);
}// namespace dice::tests::sparql

#endif//TENTRIS_TESTS_TESTS_COMMONS_HPP
