#include "MetallVariableBackend.hpp"

namespace dice::metall_node_storage {

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallVariableBackend::IdGen::next_id([[maybe_unused]] View const &view) noexcept {
		auto const id = next_id_++;
		if (id.value() >= (1ul << rdf4cpp::rdf::storage::node::identifier::NodeID::width)) [[unlikely]] {
			std::terminate();
		}

		return id;
	}

	MetallVariableBackend::MetallVariableBackend(View const &view,
												 defs::allocator_type const &allocator) noexcept : hash_{view.hash()},
																								   name{view.name, allocator},
																								   is_anonymous{view.is_anonymous} {
	}

	MetallVariableBackend::operator View() const noexcept {
		return View{.name = this->name,
					.is_anonymous = this->is_anonymous};
	}

}// namespace dice::metall_node_storage
