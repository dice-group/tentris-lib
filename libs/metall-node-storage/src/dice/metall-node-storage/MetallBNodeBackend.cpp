#include "MetallBNodeBackend.hpp"

namespace dice::metall_node_storage {

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallBNodeBackend::IdGen::next_id([[maybe_unused]] View const &view) noexcept {
		auto const id = next_id_++;
		if (id.value() >= (1ul << rdf4cpp::rdf::storage::node::identifier::NodeID::width)) [[unlikely]] {
			std::terminate();
		}

		return id;
	}

	MetallBNodeBackend::MetallBNodeBackend(View const &view,
										   defs::allocator_type const &allocator) noexcept : hash_{view.hash()},
																							 identifier{view.identifier, allocator} {
	}

	MetallBNodeBackend::operator View() const noexcept {
		return View{.identifier = this->identifier};
	}

}// namespace dice::metall_node_storage