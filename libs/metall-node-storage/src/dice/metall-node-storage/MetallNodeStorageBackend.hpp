#ifndef TENTRIS_PERSISTENTNODESTORAGEBACKEND_HPP
#define TENTRIS_PERSISTENTNODESTORAGEBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/INodeStorageBackend.hpp>
#include <dice/metall-node-storage/MetallNodeStorageBackendImpl.hpp>

namespace dice::metall_node_storage {

	struct MetallNodeStorageBackend : rdf4cpp::rdf::storage::node::INodeStorageBackend {
	private:
		MetallNodeStorageBackendImpl *impl_;

		/**
		 * @brief destroys impl_ if this->in_memory()
		 */
		void drop() noexcept;

	public:
		/**
		 * @brief Makes an in-memory instance of MetallNodeStorageBackend.
		 * @details All resources will be freed upon calling the destructor.
		 * @return in-memory instance
		 */
		explicit MetallNodeStorageBackend(defs::InMemoryFlag) noexcept;

		/**
		 * @brief Finds or constructs a persistent MetallNodeStorageBackend instance.
		 * @details
		 * @note If creation fails, an exception will be thrown. No memory is allocated in then.
		 * @note The lifetime of the storage is managed by the manager that is passed as argument.
		 * @note Only one instance per name must be exist at any point in time.
		 * @param manager The metall_manager used to find or construct the instance
		 * @param name The name of the NodeStorage
		 * @return persistent MetallNodeStorageBackend instance
		 */
		MetallNodeStorageBackend(defs::PersistentFlag, defs::metall_manager &manager, std::string const &name);

		MetallNodeStorageBackend(MetallNodeStorageBackend const &other) = delete;
		MetallNodeStorageBackend &operator=(MetallNodeStorageBackend const &other) = delete;

		MetallNodeStorageBackend(MetallNodeStorageBackend &&other) noexcept;
		MetallNodeStorageBackend &operator=(MetallNodeStorageBackend &&other) noexcept;

		~MetallNodeStorageBackend() override;

		[[nodiscard]] size_t size() const noexcept override;

		[[nodiscard]] bool has_specialized_storage_for(rdf4cpp::rdf::storage::node::identifier::LiteralType datatype) const noexcept override;

		rdf4cpp::rdf::storage::node::identifier::NodeID find_or_make_id(const rdf4cpp::rdf::storage::node::view::BNodeBackendView &view) noexcept override;
		rdf4cpp::rdf::storage::node::identifier::NodeID find_or_make_id(const rdf4cpp::rdf::storage::node::view::IRIBackendView &view) noexcept override;
		rdf4cpp::rdf::storage::node::identifier::NodeID find_or_make_id(const rdf4cpp::rdf::storage::node::view::LiteralBackendView &view) noexcept override;
		rdf4cpp::rdf::storage::node::identifier::NodeID find_or_make_id(const rdf4cpp::rdf::storage::node::view::VariableBackendView &view) noexcept override;

		rdf4cpp::rdf::storage::node::identifier::NodeID find_id(const rdf4cpp::rdf::storage::node::view::BNodeBackendView &view) const noexcept override;
		rdf4cpp::rdf::storage::node::identifier::NodeID find_id(const rdf4cpp::rdf::storage::node::view::IRIBackendView &view) const noexcept override;
		rdf4cpp::rdf::storage::node::identifier::NodeID find_id(const rdf4cpp::rdf::storage::node::view::LiteralBackendView &view) const noexcept override;
		rdf4cpp::rdf::storage::node::identifier::NodeID find_id(const rdf4cpp::rdf::storage::node::view::VariableBackendView &view) const noexcept override;

		rdf4cpp::rdf::storage::node::view::IRIBackendView find_iri_backend_view(rdf4cpp::rdf::storage::node::identifier::NodeID id) const override;
		rdf4cpp::rdf::storage::node::view::LiteralBackendView find_literal_backend_view(rdf4cpp::rdf::storage::node::identifier::NodeID id) const override;
		rdf4cpp::rdf::storage::node::view::BNodeBackendView find_bnode_backend_view(rdf4cpp::rdf::storage::node::identifier::NodeID id) const override;
		rdf4cpp::rdf::storage::node::view::VariableBackendView find_variable_backend_view(rdf4cpp::rdf::storage::node::identifier::NodeID id) const override;

		bool erase_iri(rdf4cpp::rdf::storage::node::identifier::NodeID id) noexcept override;
		bool erase_literal(rdf4cpp::rdf::storage::node::identifier::NodeID id) noexcept override;
		bool erase_bnode(rdf4cpp::rdf::storage::node::identifier::NodeID id) noexcept override;
		bool erase_variable(rdf4cpp::rdf::storage::node::identifier::NodeID id) noexcept override;

		/**
		 * @return If this is in-memory.
		 */
		[[nodiscard]] bool is_in_memory() const noexcept;

		/**
		 * @return If this is transparently persisted to disc.
		 */
		[[nodiscard]] bool is_persistent() const noexcept;

		/**
		 * @brief Destroys an previously created MetallNodeStorageBackend instance.
		 * @details Calls the destructor and frees the memory of MetallNodeStorageBackendImpl.
		 * @note
		 * If T's destructor throws:
		 * 1) the exception will be thrown (propagated);
		 * 2) the memory will won't be freed;
		 * 3) the object entry will be still removed from the attributed object directory.
		 * Therefore, it is not recommended to throw exception in a destructor.
		 * @param manager An metall_manager that was used to create the instance.
		 * @param name Name of the instance to be destroyed.
		 * @return Returns false if the object was not destroyed.
		 */
		static bool destroy_persistent(tentris::defs::metall_manager &manager, std::string const &name);
	};
}// namespace dice::metall_node_storage

#endif//TENTRIS_PERSISTENTNODESTORAGEBACKEND_HPP
