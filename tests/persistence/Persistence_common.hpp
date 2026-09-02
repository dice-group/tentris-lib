#ifndef NODESTORAGEPERSISTENCE_COMMON_HPP
#define NODESTORAGEPERSISTENCE_COMMON_HPP

#include <cassert>
#include <iostream>

#include "dice/triplestore/TripleStore.hpp"

inline constexpr char const *triplestore_name = "test-triplestore";
inline constexpr char const *test_subj = "http://example.com/testing-persistence#subj";
inline constexpr char const *test_pred = "http://example.com/testing-persistence#pred";
inline constexpr char const *test_obj = "testing-persistence";

void test_match(dice::triplestore::TripleStore const &ts) {
	using namespace rdf4cpp::rdf;

	auto const res = ts.match(query::TriplePattern{Node{},
												   IRI{test_pred},
												   Node{}});

	assert(res.size() == 1);
	auto const res_0 = *res.begin();
	auto const subj = res_0[0].as_iri();
	auto const obj = res_0[1].as_literal();

	std::cout << "subject iri: " << subj << std::endl;
	std::cout << "object literal: " << obj << std::endl;

	assert(subj == IRI{test_subj});
	assert(obj == Literal::make_simple(test_obj));
}

#endif //NODESTORAGEPERSISTENCE_COMMON_HPP
