#include "Persistence_common.hpp"

int main(int argc, char **argv) {
	using namespace rdf4cpp::rdf;
	assert(argc >= 2);

	auto const *path = argv[1];
	{
		dice::tentris::defs::metall_manager manager{metall::create_only, path};
	}

	dice::tentris::defs::metall_manager manager{metall::open_only, path};
	dice::triplestore::TripleStore ts{dice::tentris::defs::persistent, manager, triplestore_name};
	assert(ts.size() == 0);

	ts.add_statement(Statement{IRI{test_subj},
							   IRI{test_pred},
							   Literal::make_simple(test_obj)});
	ts.flush();

	std::cout << "Triplestore: " << static_cast<std::string>(ts.hypertrie()) << std::endl;

	assert(ts.size() == 1);
	test_match(ts);

	std::cout << std::endl;
}
