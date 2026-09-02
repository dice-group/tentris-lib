#ifndef RDF4CPP_METALLNODETYPESTORAGE_HPP
#define RDF4CPP_METALLNODETYPESTORAGE_HPP

#include <dice/sparse-map/sparse_map.hpp>

#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>

#include <shared_mutex>


namespace dice::metall_node_storage {
	/**
	 * Storage for one of the Node Backend types. Includes a shared mutex to synchronize access and bidirectional mappings between the Backend type and identifier::NodeID.
	 * @tparam BackendType_t one of BNodeBackend, IRIBackend, LiteralBackend and VariableBackend.
	 */
	template<typename BackendType_t>
	struct MetallNodeTypeStorage {
		using allocator_type = defs::allocator_type;
		using Backend = BackendType_t;
        using BackendView = typename Backend::View;
		using BackendIdGen = typename Backend::IdGen;
		using Backend_allocator_type = ::dice::tentris::defs::allocator_type_t<Backend>;
		using Backend_ptr = typename Backend_allocator_type::pointer;
		using Backend_const_ptr = typename Backend_allocator_type::const_pointer;

	private:
		struct DefaultBackendTypeEqual {
			using is_transparent = void;

			bool operator()(Backend_const_ptr lhs, Backend_const_ptr rhs) const noexcept {
				return lhs == rhs;
			}
			bool operator()(BackendView const &lhs, Backend_const_ptr rhs) const noexcept {
				return lhs == BackendView(*rhs);
			}

			bool operator()(Backend_const_ptr lhs, BackendView const &rhs) const noexcept {
				return BackendView(*lhs) == rhs;
			}
		};

		template<typename T>
		struct SelectBackendTypeEqual {
			using type = DefaultBackendTypeEqual;
		};

		template<typename T> requires requires { typename T::Equal; }
		struct SelectBackendTypeEqual<T> {
			using type = typename T::Equal;
		};

		struct DefaultBackendTypeHash {
			[[nodiscard]] size_t operator()(Backend_const_ptr x) const noexcept {
				return x->hash();
			}
			[[nodiscard]] size_t operator()(BackendView const &x) const noexcept {
				return x.hash();
			}
		};

		template<typename T>
		struct SelectBackendTypeHash {
			using type = DefaultBackendTypeHash;
		};

		template<typename T> requires requires { typename T::Hash; }
		struct SelectBackendTypeHash<T> {
			using type = typename T::Hash;
		};

	public:
		using BackendTypeEqual = typename SelectBackendTypeEqual<BackendType_t>::type;
		using BackendTypeHash = typename SelectBackendTypeHash<BackendType_t>::type;

		struct NodeIDHash {
			[[nodiscard]] size_t operator()(rdf4cpp::rdf::storage::node::identifier::NodeID const &x) const noexcept {
				return x.value();
			}
		};

		std::shared_mutex mutable mutex; // synchronizes access to id_gen, id2data and data2id

		Backend_allocator_type backend_allocator;

		BackendIdGen id_gen;

		dice::sparse_map::sparse_map<rdf4cpp::rdf::storage::node::identifier::NodeID, Backend_ptr, NodeIDHash, std::equal_to<>,
									 defs::allocator_type_t<std::pair<rdf4cpp::rdf::storage::node::identifier::NodeID, Backend_ptr>>>
				id2data;

		dice::sparse_map::sparse_map<Backend_ptr, rdf4cpp::rdf::storage::node::identifier::NodeID, BackendTypeHash, BackendTypeEqual,
									 defs::allocator_type_t<std::pair<Backend_ptr, rdf4cpp::rdf::storage::node::identifier::NodeID>>>
				data2id;

		/**
		 * Yes, this constructor has no definition and therefore doesn't even link when explicitly called.
		 *
		 * But for _some reason_ std::tuple requires all types to be *default* constructible when using
		 * std::tuple::tuple(std::allocator_arg_t, Alloc). Which is why this ctor is declared here.
		 *
		 * We don't even want it to compile if you call this ctor because the only
		 * way to default construct this type would be providing a default-constructed allocator_type
		 * which could potentially break persistence if this ctor was accidentally called.
		 */
		MetallNodeTypeStorage();

		explicit MetallNodeTypeStorage(allocator_type const &alloc) : backend_allocator{alloc},
																	  id2data{alloc},
																	  data2id{alloc} {
		}

		~MetallNodeTypeStorage() noexcept {
			for (auto const &[_, backend] : id2data) {
				backend->~Backend();
				backend_allocator.deallocate(backend, 1);
			}
		}
	};
}// namespace dice::metall_node_storage

#endif//RDF4CPP_METALLNODETYPESTORAGE_HPP
