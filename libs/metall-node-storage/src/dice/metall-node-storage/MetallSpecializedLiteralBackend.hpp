#ifndef TENTRIS_METALLSPECIALIZEDLITERALBACKEND_HPP
#define TENTRIS_METALLSPECIALIZEDLITERALBACKEND_HPP

#include <rdf4cpp/rdf/storage/node/identifier/NodeID.hpp>
#include <rdf4cpp/rdf/storage/node/view/LiteralBackendView.hpp>

#include <dice/metall-node-storage/defs.hpp>

namespace dice::metall_node_storage {

	template<rdf4cpp::rdf::datatypes::FixedIdLiteralDatatype T>
	struct MetallSpecializedLiteralBackend {
		using View = rdf4cpp::rdf::storage::node::view::ValueLiteralBackendView;
		using Type = T;

		using SpecializedLiteralBackend_allocator_type = defs::metall_manager::allocator_type<MetallSpecializedLiteralBackend>;
		using SpecializedLiteralBackend_const_pointer = typename SpecializedLiteralBackend_allocator_type::const_pointer;

		struct IdGen {
		private:
			using LiteralType = rdf4cpp::rdf::storage::node::identifier::LiteralType;
			using LiteralID = rdf4cpp::rdf::storage::node::identifier::LiteralID;
			static constexpr LiteralID min_id = rdf4cpp::rdf::storage::node::identifier::NodeID::min_literal_id;

			LiteralID next_id_ = min_id;

		public:
			IdGen() noexcept = default;

			[[nodiscard]] rdf4cpp::rdf::storage::node::identifier::NodeID next_id([[maybe_unused]] View const &view) noexcept {
				auto const id = next_id_++;
				if (id.to_underlying() >= (1ul << LiteralID::width)) [[unlikely]] {
					std::terminate();
				}

				return rdf4cpp::rdf::storage::node::identifier::NodeID{id, datatype};
			}
		};

	private:
		size_t hash_;

	public:
		static constexpr rdf4cpp::rdf::storage::node::identifier::LiteralType datatype = T::fixed_id;
		typename T::cpp_type value;

		MetallSpecializedLiteralBackend(View const &view, [[maybe_unused]] defs::allocator_type const &alloc) noexcept : hash_{view.hash<T>()},
																														 value{std::any_cast<typename T::cpp_type>(view.value)} {
			assert(view.datatype == MetallSpecializedLiteralBackend::datatype);
		}

		[[nodiscard]] size_t hash() const noexcept {
			return hash_;
		}

		explicit operator View() const noexcept {
			return View{.datatype = MetallSpecializedLiteralBackend::datatype,
						.value = std::any{this->value}};
		}

	public:
		struct Equal {
			using is_transparent = void;

			[[nodiscard]] bool operator()(SpecializedLiteralBackend_const_pointer lhs, SpecializedLiteralBackend_const_pointer rhs) const noexcept {
				return lhs == rhs;
			}

			[[nodiscard]] bool operator()(View const &lhs, SpecializedLiteralBackend_const_pointer rhs) const noexcept {
				assert(lhs.datatype == MetallSpecializedLiteralBackend::datatype);
				return std::any_cast<typename T::cpp_type>(lhs.value) == rhs->value;
			}

			[[nodiscard]] bool operator()(SpecializedLiteralBackend_const_pointer lhs, View const &rhs) const noexcept {
				assert(MetallSpecializedLiteralBackend::datatype == rhs.datatype);
				return lhs->value == std::any_cast<typename T::cpp_type>(rhs.value);
			}
		};

		struct Hash {
			[[nodiscard]] size_t operator()(SpecializedLiteralBackend_const_pointer x) const noexcept {
				return x->hash();
			}

			[[nodiscard]] size_t operator()(View const &x) const noexcept {
				assert(x.datatype == MetallSpecializedLiteralBackend::datatype);
				return x.hash<T>();
			}
		};
	};

} // namespace dice::metall_node_storage


#endif//TENTRIS_METALLSPECIALIZEDLITERALBACKEND_HPP
