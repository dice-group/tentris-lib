#ifndef RDF4CPP_METALLIRIBACKEND_HPP
#define RDF4CPP_METALLIRIBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>
#include <rdf4cpp/rdf/storage/node/view/IRIBackendView.hpp>

#include <dice/metall-node-storage/defs.hpp>

namespace dice::metall_node_storage {
	struct MetallIRIBackend {
		using View = rdf4cpp::rdf::storage::node::view::IRIBackendView;

		struct IdGen {
		private:
			using NodeID = rdf4cpp::rdf::storage::node::identifier::NodeID;
			static constexpr NodeID min_id = NodeID::min_iri_id;

			NodeID next_id_ = min_id;

		public:
			IdGen() noexcept = default;
			[[nodiscard]] rdf4cpp::rdf::storage::node::identifier::NodeID next_id([[maybe_unused]] View const &view) noexcept;
		};

	private:
		size_t hash_;

	public:
		defs::metall_string identifier;

		MetallIRIBackend(View const &view, defs::allocator_type const &allocator) noexcept;

		[[nodiscard]] size_t hash() const noexcept { return hash_; }

		explicit operator View() const noexcept;
	};

}// namespace dice::metall_node_storage


#endif//RDF4CPP_METALLIRIBACKEND_HPP
