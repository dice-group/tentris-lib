#ifndef RDF4CPP_METALLLITERALBACKEND_HPP
#define RDF4CPP_METALLLITERALBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>
#include <rdf4cpp/rdf/storage/node/view/LiteralBackendView.hpp>

#include <dice/metall-node-storage/defs.hpp>

namespace dice::metall_node_storage {

	struct MetallFallbackLiteralBackend {
		using View = rdf4cpp::rdf::storage::node::view::LexicalFormLiteralBackendView;

		struct IdGen {
		private:
			using LiteralType = rdf4cpp::rdf::storage::node::identifier::LiteralType;
			using LiteralID = rdf4cpp::rdf::storage::node::identifier::LiteralID;
			static constexpr LiteralID min_id = rdf4cpp::rdf::storage::node::identifier::NodeID::min_literal_id;

			std::array<LiteralID, 1ul << LiteralType::width> next_ids_;

		public:
			IdGen() noexcept;
			[[nodiscard]] rdf4cpp::rdf::storage::node::identifier::NodeID next_id(View const &view) noexcept;
		};

	private:
		size_t hash_;

	public:
		rdf4cpp::rdf::storage::node::identifier::NodeID datatype_id;
		defs::metall_string lexical_form;
		defs::metall_string language_tag;
		bool needs_escape;

		MetallFallbackLiteralBackend(View const &view,
									 defs::allocator_type const &allocator) noexcept;

		[[nodiscard]] inline size_t hash() const noexcept { return hash_; }

		explicit operator View() const noexcept;
	};

}// namespace dice::metall_node_storage


#endif//RDF4CPP_METALLLITERALBACKEND_HPP
