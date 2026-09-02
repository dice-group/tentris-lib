#ifndef TENTRIS_LIB_PARAMS_HPP
#define TENTRIS_LIB_PARAMS_HPP

#include <metall/basic_manager.hpp>
#define DICE_TEMPLATE_LIBRARY_WITH_BOOST
#include <dice/template-library/polymorphic_allocator.hpp>

namespace dice::tentris::defs {
	struct PersistentFlag {};
	inline constexpr PersistentFlag persistent;

	struct InMemoryFlag {};
	inline constexpr InMemoryFlag in_memory;

	template<typename T>
	using std_allocator_type_t = template_library::offset_ptr_stl_allocator<T>;
	using std_allocator_type = std_allocator_type_t<std::byte>;

	using metall_manager = metall::basic_manager<uint32_t, (1ULL << 28ULL)>;

	template<typename T>
	using metall_allocator_type_t = metall_manager::allocator_type<T>;
	using metall_allocator_type = metall_allocator_type_t<std::byte>;

	template<typename T>
	using allocator_type_t = template_library::polymorphic_allocator<T, std_allocator_type_t, metall_allocator_type_t>;
	using allocator_type = allocator_type_t<std::byte>;
}// namespace dice::tentris::defs

#endif//TENTRIS_LIB_PARAMS_HPP
