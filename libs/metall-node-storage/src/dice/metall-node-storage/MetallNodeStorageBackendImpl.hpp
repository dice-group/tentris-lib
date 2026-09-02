#ifndef TENTRIS_PERSISTENTNODESTORAGEBACKENDIMPL_HPP
#define TENTRIS_PERSISTENTNODESTORAGEBACKENDIMPL_HPP

#include "dice/metall-node-storage/MetallBNodeBackend.hpp"
#include "dice/metall-node-storage/MetallFallbackLiteralBackend.hpp"
#include "dice/metall-node-storage/MetallIRIBackend.hpp"
#include "dice/metall-node-storage/MetallNodeTypeStorage.hpp"
#include "dice/metall-node-storage/MetallVariableBackend.hpp"
#include "dice/metall-node-storage/MetallSpecializedLiteralBackend.hpp"


namespace dice::metall_node_storage {

	struct MetallNodeStorageBackendImpl {
        using allocator_type = defs::allocator_type;

    private:
		using NodeID = rdf4cpp::rdf::storage::node::identifier::NodeID;
		using LiteralType = rdf4cpp::rdf::storage::node::identifier::LiteralType;

		using LiteralBackendView = rdf4cpp::rdf::storage::node::view::LiteralBackendView;
		using BNodeBackendView = rdf4cpp::rdf::storage::node::view::BNodeBackendView;
		using IRIBackendView = rdf4cpp::rdf::storage::node::view::IRIBackendView;
		using VariableBackendView = rdf4cpp::rdf::storage::node::view::VariableBackendView;

	private:
		MetallNodeTypeStorage<MetallBNodeBackend> bnode_storage_;
		MetallNodeTypeStorage<MetallIRIBackend> iri_storage_;
		MetallNodeTypeStorage<MetallVariableBackend> variable_storage_;

		MetallNodeTypeStorage<MetallFallbackLiteralBackend> fallback_literal_storage_;

		std::tuple<MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::Long>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::UnsignedLong>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::Double>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::Date>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::DateTime>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::DateTimeStamp>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::GYearMonth>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::Duration>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::DayTimeDuration>>,
				   MetallNodeTypeStorage<MetallSpecializedLiteralBackend<rdf4cpp::rdf::datatypes::xsd::YearMonthDuration>>> specialized_literal_storage_;

		// TODO: The following types need allocator awareness in rdf4cpp to work properly
		// datatypes::xsd::Integer
		// datatypes::xsd::NonNegativeInteger
		// datatypes::xsd::PositiveInteger
		// datatypes::xsd::NonPositiveInteger
		// datatypes::xsd::NegativeInteger
		// datatypes::xsd::Decimal
		// datatypes::xsd::Base64Binary
		// datatypes::xsd::HexBinary

		/**
		 * @tparam Datatype datatype of the specialized storage
		 * @param self *this
		 * @return corresponding specialized storage for the given datatype
		 */
		template<rdf4cpp::rdf::datatypes::FixedIdLiteralDatatype Datatype, typename Self>
		static decltype(auto) get_specialized(Self &&self) noexcept;

		/**
		 * Calls the given function f with the specialized object for the given datatype
		 *
		 * @param self *this
		 * @param datatype the datatype of the specialized object
		 * @param f the function to call with the corresponding specialized object
		 * @return whatever f returns
		 */
		template<typename Self, typename F>
		static decltype(auto) visit_specialized(Self &&self, rdf4cpp::rdf::storage::node::identifier::LiteralType datatype, F f);

	public:
		explicit MetallNodeStorageBackendImpl(defs::allocator_type const &alloc);

		[[nodiscard]] size_t size() const noexcept;

		[[nodiscard]] bool has_specialized_storage_for(LiteralType datatype) const noexcept;

		[[nodiscard]] NodeID find_or_make_id(BNodeBackendView const &) noexcept;
		[[nodiscard]] NodeID find_or_make_id(IRIBackendView const &) noexcept;
		[[nodiscard]] NodeID find_or_make_id(LiteralBackendView const &) noexcept;
		[[nodiscard]] NodeID find_or_make_id(VariableBackendView const &) noexcept;

		[[nodiscard]] NodeID find_id(BNodeBackendView const &) const noexcept;
		[[nodiscard]] NodeID find_id(IRIBackendView const &) const noexcept;
		[[nodiscard]] NodeID find_id(LiteralBackendView const &) const noexcept;
		[[nodiscard]] NodeID find_id(VariableBackendView const &) const noexcept;

		[[nodiscard]] IRIBackendView find_iri_backend_view(NodeID id) const;
		[[nodiscard]] LiteralBackendView find_literal_backend_view(NodeID id) const;
		[[nodiscard]] BNodeBackendView find_bnode_backend_view(NodeID id) const;
		[[nodiscard]] VariableBackendView find_variable_backend_view(NodeID id) const;

		bool erase_iri(NodeID id) noexcept;
		bool erase_literal(NodeID id) noexcept;
		bool erase_bnode(NodeID id) noexcept;
		bool erase_variable(NodeID id) noexcept;

		bool is_in_memory() const noexcept;
		bool is_persistent() const noexcept;
	};

}// namespace dice::metall_node_storage

#endif//TENTRIS_PERSISTENTNODESTORAGEBACKENDIMPL_HPP
