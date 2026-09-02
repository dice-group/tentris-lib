#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "dice/ffi/tentris.h"

#include <cerrno>
#include <string>
#include <fstream>
#include <random>
#include <vector>

std::vector<std::string> read_lines(std::string const &path) {
	std::ifstream ifs{path};

	std::vector<std::string> ret;

	std::string buf;
	while (std::getline(ifs, buf)) {
		ret.push_back(buf);
	}

	return ret;
}

/**
 * reinterprets data as std::string and calls append(bytes, size) on it
 */
void buf_sink(void *data, char const *bytes, size_t size) {
	reinterpret_cast<std::string *>(data)->append(bytes, size);
}

/**
 * Issues a SELECT (COUNT(*) as ?ntriples) WHERE { ?s ?p ?o } query and checks its result
 */
void check_count_query(tentris_triplestore const *ts, size_t expected_size) {
	tentris_error error;
	tentris_solution_generator gen;
	if (tentris_triplestore_eval_query(ts, "SELECT (COUNT(*) as ?ntriples) WHERE { ?s ?p ?o }", UINT64_MAX, &gen, &error) != TENTRIS_SUCCESS) {
		FAIL("Query failed: " << tentris_strerror(&error));
	}

	rdf4cpp_variable const *proj_vars;
	size_t proj_vars_size;
	tentris_solution_generator_get_projected_variables(&gen, &proj_vars, &proj_vars_size);
	CHECK_EQ(proj_vars_size, 1);
	CHECK_EQ(rdf4cpp_type_of_term(proj_vars[0]), RDF4CPP_TT_VARIABLE);
	CHECK_EQ(rdf4cpp_variable_name(proj_vars[0]), std::string{"ntriples"});

	tentris_solution s;
	size_t count = 0;
	while (true) {
		tentris_iter_result const res = tentris_solution_generator_next(&gen, &s, &error);
		switch (res) {
			case TENTRIS_I_YIELDED: {
				CHECK_EQ(s.count, 1);
				CHECK_EQ(s.key_len, 1);
				CHECK_EQ(rdf4cpp_type_of_term(s.key[0]), RDF4CPP_TT_LITERAL);

				char const *lex_form;
				rdf4cpp_literal_lexical_form(s.key[0], &lex_form);

				CHECK_EQ(strlen(lex_form), 2);

				CHECK_EQ(std::string{lex_form}, std::to_string(expected_size));
				CHECK_EQ(rdf4cpp_iri_identifier(rdf4cpp_literal_datatype(s.key[0])), std::string{"http://www.w3.org/2001/XMLSchema#integer"});

				count += 1;
				continue;
			}
			case TENTRIS_I_FAILURE: {
				FAIL(tentris_strerror(&error));
			}
			case TENTRIS_I_ENDED: {
			}
		}

		break;
	}

	CHECK_EQ(count, 1);
	tentris_solution_generator_destroy(&gen);
}

TEST_SUITE("ffi") {
	TEST_CASE("iterate and serialize") {
		tentris_triplestore ts;
		tentris_triplestore_init_in_memory(&ts);
		tentris_triplestore_load_ttl(&ts, "./swdf_10.nt");

		auto stmts = read_lines("./swdf_10.nt");
		REQUIRE_EQ(tentris_triplestore_size(&ts), stmts.size());

		tentris_triplestore_iterator iter;
		tentris_triplestore_iterate(&ts, &iter);

		std::vector<std::string> bufs;

		tentris_triple const *t;
		while (tentris_triplestore_iterator_next(&iter, &t) != TENTRIS_I_ENDED) {
			auto &buf = bufs.emplace_back();

			rdf4cpp_serialize(t->key[0], buf_sink, &buf);
			buf_sink(&buf, " ", 1);
			rdf4cpp_serialize(t->key[1], buf_sink, &buf);
			buf_sink(&buf, " ", 1);
			rdf4cpp_serialize(t->key[2], buf_sink, &buf);
			buf_sink(&buf, " .", 2);
		}

		tentris_triplestore_iterator_destroy(&iter);

		std::sort(stmts.begin(), stmts.end());
		std::sort(bufs.begin(), bufs.end());

		REQUIRE_EQ(bufs.size(), stmts.size());
		for (size_t ix = 0; ix < bufs.size(); ++ix) {
			REQUIRE_EQ(bufs[ix], stmts[ix]);
		}

		tentris_triplestore_destroy(&ts);
	}

	TEST_CASE("full query integration test") {
		tentris_triplestore ts;
		tentris_triplestore_init_in_memory(&ts);

		if (tentris_triplestore_load_ttl(&ts, "./swdf_10.nt") != TENTRIS_SUCCESS) {
			FAIL("Loading data failed: " << strerror(errno));
		}

		check_count_query(&ts, 10);

		tentris_triplestore_destroy(&ts);
	}

	TEST_CASE("update sanity check") {
		tentris_triplestore ts;
		tentris_triplestore_init_in_memory(&ts);

		tentris_error err;
		if (tentris_triplestore_eval_update(&ts, "INSERT DATA { <https://url.com#subj> <https://url.com#pred> <https://url.com#obj> . }", TENTRIS_NO_TIMEOUT, &err) != TENTRIS_SUCCESS) {
			FAIL("INSERT DATA failed: " << tentris_strerror(&err));
		}

		CHECK_EQ(tentris_triplestore_size(&ts), 1);

		tentris_triplestore_destroy(&ts);
	}

	TEST_CASE("dedicated update sanity check") {
		tentris_triplestore ts;
		tentris_triplestore_init_in_memory(&ts);

		tentris_error err;
		if (tentris_triplestore_eval_insert_or_delete_data(&ts, "INSERT DATA { <https://url.com#subj> <https://url.com#pred> <https://url.com#obj> . }", TENTRIS_NO_TIMEOUT, &err) != TENTRIS_SUCCESS) {
			FAIL("INSERT DATA failed: " << tentris_strerror(&err));
		}

		CHECK_EQ(tentris_triplestore_size(&ts), 1);

		tentris_triplestore_destroy(&ts);
	}

	TEST_CASE("metall persistence") {
		auto const datastore_path = "/tmp/tentris_tests_ffi" + std::to_string(std::random_device{}());
		char const *triplestore_name = "ffi_triplestore";

		{
			metall_manager *manager = metall_create(datastore_path.c_str());
			if (manager == nullptr) {
				FAIL("Creation of datastore failed: " << strerror(errno));
			}

			tentris_triplestore pts;
			if (tentris_triplestore_find_or_construct_persistent(&pts, manager, triplestore_name) != TENTRIS_SUCCESS) {
				FAIL("Creation of triplestore failed: " << strerror(errno));
			}

			tentris_triplestore_load_ttl(&pts, "./swdf_10.nt");

			tentris_triplestore_destroy(&pts);
			metall_close(manager);
		}

		{
			metall_manager *manager = metall_open(datastore_path.c_str());
			if (manager == nullptr) {
				FAIL("Opening of datastore failed: " << strerror(errno));
			}

			tentris_triplestore pts;
			if (tentris_triplestore_find_or_construct_persistent(&pts, manager, triplestore_name) != TENTRIS_SUCCESS) {
				FAIL("Opening of triplestore failed: " << strerror(errno));
			}

			check_count_query(&pts, 10);

			tentris_triplestore_destroy(&pts);
			metall_close(manager);
			metall_remove(datastore_path.c_str());
		}
	}
}
