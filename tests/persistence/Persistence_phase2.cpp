#include "Persistence_common.hpp"

int main(int argc, char **argv) {
	using namespace rdf4cpp::rdf;
	assert(argc >= 2);

	auto const *path = argv[1];

	dice::tentris::defs::metall_manager manager{metall::open_only, path};
	dice::triplestore::TripleStore ts{dice::tentris::defs::persistent, manager, triplestore_name};

	auto const reconstruct_size = ts.size();
	std::cout << "Size after reconstructing from persistence: " << reconstruct_size << std::endl;
	assert(reconstruct_size == 1);

	std::cout << "Reconstructed Triplestore: " << static_cast<std::string>(ts.hypertrie()) << std::endl;

	test_match(ts);
	assert(ts.size() == 1);
}
