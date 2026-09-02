#ifndef TENTRIS_METALL_NODE_STORAGE_DEFS_HPP
#define TENTRIS_METALL_NODE_STORAGE_DEFS_HPP


#ifndef BOOST_BIND_GLOBAL_PLACEHOLDERS
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#endif

#include <dice/tentris/param_allocator.hpp>

#include <metall/container/string.hpp>

namespace dice::metall_node_storage::defs {
	using namespace dice::tentris::defs;

	using metall_string = metall::container::basic_string<char, std::char_traits<char>, allocator_type_t<char>>;
}// namespace dice::metall_node_storage::defs
#endif//TENTRIS_METALL_NODE_STORAGE_DEFS_HPP
