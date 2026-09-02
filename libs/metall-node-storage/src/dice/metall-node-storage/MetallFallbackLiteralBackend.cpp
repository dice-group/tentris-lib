#include "MetallFallbackLiteralBackend.hpp"

namespace dice::metall_node_storage {
	MetallFallbackLiteralBackend::IdGen::IdGen() noexcept {
		next_ids_.fill(min_id);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallFallbackLiteralBackend::IdGen::next_id(View const &view) noexcept {
		auto const datatype = rdf4cpp::rdf::storage::node::identifier::iri_node_id_to_literal_type(view.datatype_id);
		auto const id = next_ids_[datatype.to_underlying()]++;
		if (id.to_underlying() >= (1ul << LiteralID::width)) [[unlikely]] {
			std::terminate();
		}

		return rdf4cpp::rdf::storage::node::identifier::NodeID{id, datatype};
	}

	MetallFallbackLiteralBackend::MetallFallbackLiteralBackend(View const &view,
															   defs::allocator_type const &allocator) noexcept : hash_{view.hash()},
																												 datatype_id{view.datatype_id},
																												 lexical_form{view.lexical_form, allocator},
																												 language_tag{view.language_tag, allocator},
																												 needs_escape{view.needs_escape} {
	}

	MetallFallbackLiteralBackend::operator View() const noexcept {
		return View{.datatype_id = this->datatype_id,
					.lexical_form = this->lexical_form,
					.language_tag = this->language_tag,
					.needs_escape = this->needs_escape};
	}

}// namespace dice::metall_node_storage
