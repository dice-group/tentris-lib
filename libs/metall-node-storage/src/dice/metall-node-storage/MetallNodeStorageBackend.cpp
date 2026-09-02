#include "MetallNodeStorageBackend.hpp"

namespace dice::metall_node_storage {

	inline constexpr char const *node_storage_suffix = "node-storage";

	void MetallNodeStorageBackend::drop() noexcept {
		if (is_in_memory()) {
			delete impl_;
		}
	}

	MetallNodeStorageBackend::MetallNodeStorageBackend(defs::InMemoryFlag) noexcept
		: impl_{new MetallNodeStorageBackendImpl{::dice::tentris::defs::allocator_type{::dice::tentris::defs::std_allocator_type{}}}} {
	}

	MetallNodeStorageBackend::MetallNodeStorageBackend(defs::PersistentFlag, defs::metall_manager &manager, std::string const &name) {
		auto const node_storage_name = name + node_storage_suffix;
		impl_ = manager.find_or_construct<MetallNodeStorageBackendImpl>(node_storage_name.c_str())(::dice::tentris::defs::allocator_type{manager.get_allocator()});
	}

	MetallNodeStorageBackend::MetallNodeStorageBackend(MetallNodeStorageBackend &&other) noexcept
		: impl_{std::exchange(other.impl_, nullptr)} {
	}

	MetallNodeStorageBackend &MetallNodeStorageBackend::operator=(MetallNodeStorageBackend &&other) noexcept {
		assert(this != &other);

		drop();
		impl_ = std::exchange(other.impl_, nullptr);
		return *this;
	}

	MetallNodeStorageBackend::~MetallNodeStorageBackend() {
		drop();
	}

	size_t MetallNodeStorageBackend::size() const noexcept {
		return impl_->size();
	}

	bool MetallNodeStorageBackend::has_specialized_storage_for(rdf4cpp::rdf::storage::node::identifier::LiteralType datatype) const noexcept {
		return impl_->has_specialized_storage_for(datatype);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallNodeStorageBackend::find_or_make_id(const rdf4cpp::rdf::storage::node::view::BNodeBackendView &view) noexcept {
		return impl_->find_or_make_id(view);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallNodeStorageBackend::find_or_make_id(const rdf4cpp::rdf::storage::node::view::IRIBackendView &view) noexcept {
		return impl_->find_or_make_id(view);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallNodeStorageBackend::find_or_make_id(const rdf4cpp::rdf::storage::node::view::LiteralBackendView &view) noexcept {
		return impl_->find_or_make_id(view);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallNodeStorageBackend::find_or_make_id(const rdf4cpp::rdf::storage::node::view::VariableBackendView &view) noexcept {
		return impl_->find_or_make_id(view);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallNodeStorageBackend::find_id(const rdf4cpp::rdf::storage::node::view::BNodeBackendView &view) const noexcept {
		return impl_->find_id(view);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallNodeStorageBackend::find_id(const rdf4cpp::rdf::storage::node::view::IRIBackendView &view) const noexcept {
		return impl_->find_id(view);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallNodeStorageBackend::find_id(const rdf4cpp::rdf::storage::node::view::LiteralBackendView &view) const noexcept {
		return impl_->find_id(view);
	}

	rdf4cpp::rdf::storage::node::identifier::NodeID MetallNodeStorageBackend::find_id(const rdf4cpp::rdf::storage::node::view::VariableBackendView &view) const noexcept {
		return impl_->find_id(view);
	}

	rdf4cpp::rdf::storage::node::view::IRIBackendView MetallNodeStorageBackend::find_iri_backend_view(rdf4cpp::rdf::storage::node::identifier::NodeID id) const {
		return impl_->find_iri_backend_view(id);
	}

	rdf4cpp::rdf::storage::node::view::LiteralBackendView MetallNodeStorageBackend::find_literal_backend_view(rdf4cpp::rdf::storage::node::identifier::NodeID id) const {
		return impl_->find_literal_backend_view(id);
	}

	rdf4cpp::rdf::storage::node::view::BNodeBackendView MetallNodeStorageBackend::find_bnode_backend_view(rdf4cpp::rdf::storage::node::identifier::NodeID id) const {
		return impl_->find_bnode_backend_view(id);
	}

	rdf4cpp::rdf::storage::node::view::VariableBackendView MetallNodeStorageBackend::find_variable_backend_view(rdf4cpp::rdf::storage::node::identifier::NodeID id) const {
		return impl_->find_variable_backend_view(id);
	}

	bool MetallNodeStorageBackend::erase_iri(rdf4cpp::rdf::storage::node::identifier::NodeID id) noexcept {
		return impl_->erase_iri(id);
	}

	bool MetallNodeStorageBackend::erase_literal(rdf4cpp::rdf::storage::node::identifier::NodeID id) noexcept {
		return impl_->erase_literal(id);
	}

	bool MetallNodeStorageBackend::erase_bnode(rdf4cpp::rdf::storage::node::identifier::NodeID id) noexcept {
		return impl_->erase_bnode(id);
	}

	bool MetallNodeStorageBackend::erase_variable(rdf4cpp::rdf::storage::node::identifier::NodeID id) noexcept {
		return impl_->erase_variable(id);
	}

	bool MetallNodeStorageBackend::is_in_memory() const noexcept {
		if (impl_ == nullptr)
			return false;
		return impl_->is_in_memory();
	}

	bool MetallNodeStorageBackend::is_persistent() const noexcept {
		if (impl_ == nullptr)
			return false;
		return impl_->is_persistent();
	}

	bool MetallNodeStorageBackend::destroy_persistent(defs::metall_manager &manager, const std::string &name) {
		auto const node_storage_name = name + node_storage_suffix;
		return manager.destroy<MetallNodeStorageBackendImpl>(node_storage_name.c_str());
	}

}// namespace dice::metall_node_storage