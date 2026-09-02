#ifndef TENTRIS_FFI_SOLUTION_GENERATOR_IMPL
#define TENTRIS_FFI_SOLUTION_GENERATOR_IMPL

#include <dice/triplestore/TripleStore.hpp>

struct SolutionGeneratorImpl {
	dice::triplestore::TripleStore const *triplestore;
	std::chrono::steady_clock::time_point end_time;
	std::shared_ptr<dice::sparql::SPARQLQuery const> query;
	bool init = false;
	dice::triplestore::TripleStore::SolutionMappingGenerator generator{};
	dice::triplestore::TripleStore::SolutionMappingGenerator::iterator iter{};
};

struct IteratorImpl {
	dice::triplestore::TripleStoreIterator iter;
	bool first = true;
};

#endif // TENTRIS_FFI_SOLUTION_GENERATOR_IMPL
