#ifndef RDF4CPP_METALLVARIABLEBACKEND_HPP
#define RDF4CPP_METALLVARIABLEBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>
#include <rdf4cpp/rdf/storage/node/view/VariableBackendView.hpp>

#include <dice/metall-node-storage/defs.hpp>

namespace dice::metall_node_storage {

	struct MetallVariableBackend {
		using View = rdf4cpp::rdf::storage::node::view::VariableBackendView;

		struct IdGen {
		private:
			using NodeID = rdf4cpp::rdf::storage::node::identifier::NodeID;
			static constexpr NodeID min_id = NodeID::min_variable_id;

			NodeID next_id_ = min_id;

		public:
			IdGen() noexcept = default;
			[[nodiscard]] rdf4cpp::rdf::storage::node::identifier::NodeID next_id([[maybe_unused]] View const &view) noexcept;
		};

	private:
		size_t hash_;

	public:
		defs::metall_string name;
		bool is_anonymous;

		MetallVariableBackend(View const &view, defs::allocator_type const &allocator) noexcept;

		[[nodiscard]] size_t hash() const noexcept { return hash_; }

		explicit operator View() const noexcept;
	};

}// namespace dice::metall_node_storage


#endif//RDF4CPP_METALLVARIABLEBACKEND_HPP
