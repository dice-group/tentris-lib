#include "MetallNodeStorageBackendImpl.hpp"

#include <utility>

namespace dice::metall_node_storage {
	using namespace rdf4cpp::rdf::storage::node;

	namespace specialization_detail {

		template<typename Tuple, typename Acc, typename FoldF, size_t ...Ixs>
		constexpr Acc tuple_type_fold_impl(std::index_sequence<Ixs...>, Acc init, FoldF f) noexcept {
			((init = f.template operator()<std::tuple_element_t<Ixs, Tuple>>(std::move(init))), ...);
			return init;
		}

		template<typename Tuple, typename Acc, typename FoldF, size_t ...Ixs>
		constexpr Acc tuple_fold_impl(std::index_sequence<Ixs...>, Tuple const &tuple, Acc init, FoldF f) noexcept {
			((init = f(std::move(init), std::get<Ixs>(tuple))), ...);
			return init;
		}

		template<typename Tuple, typename Acc, typename FoldF>
		constexpr Acc tuple_type_fold(Acc &&init, FoldF &&f) noexcept {
			return tuple_type_fold_impl<Tuple>(std::make_index_sequence<std::tuple_size_v<Tuple>>{}, std::forward<Acc>(init), std::forward<FoldF>(f));
		}

		template<typename Tuple, typename Acc, typename FoldF>
		constexpr Acc tuple_fold(Tuple const &tuple, Acc &&init, FoldF &&f) noexcept {
			return tuple_fold_impl(std::make_index_sequence<std::tuple_size_v<Tuple>>{}, tuple, std::forward<Acc>(init), std::forward<FoldF>(f));
		}

		template<typename Tuple>
		static consteval std::array<bool, 1 << identifier::LiteralType::width> make_storage_specialization_lut() noexcept {
			std::array<bool, 1 << identifier::LiteralType::width> ret{};

			tuple_type_fold<Tuple>(0, [&]<typename T>(auto acc) {
				ret[T::Backend::Type::fixed_id.to_underlying()] = true;
				return acc;
			});

			return ret;
		}

	}  //specialization_detail

	template<rdf4cpp::rdf::datatypes::FixedIdLiteralDatatype Datatype, typename Self>
	decltype(auto) MetallNodeStorageBackendImpl::get_specialized(Self &&self) noexcept {
		return std::get<MetallNodeTypeStorage<MetallSpecializedLiteralBackend<Datatype>>>(std::forward<Self>(self).specialized_literal_storage_);
	}

	template<typename Self, typename F>
	decltype(auto) MetallNodeStorageBackendImpl::visit_specialized(Self &&self, identifier::LiteralType const datatype, F f) {
		using namespace rdf4cpp::rdf::datatypes;

		// manually translate runtime knowledge to compiletime
		// can probably be done using metaprogramming, but this is faster
		switch (datatype.to_underlying()) {
			case xsd::Long::fixed_id.to_underlying():
				return f(get_specialized<xsd::Long>(std::forward<Self>(self)));
			case xsd::UnsignedLong::fixed_id.to_underlying():
				return f(get_specialized<xsd::UnsignedLong>(std::forward<Self>(self)));
			case xsd::Double::fixed_id.to_underlying():
				return f(get_specialized<xsd::Double>(std::forward<Self>(self)));
			case xsd::Date::fixed_id.to_underlying():
				return f(get_specialized<xsd::Date>(std::forward<Self>(self)));
			case xsd::DateTime::fixed_id.to_underlying():
				return f(get_specialized<xsd::DateTime>(std::forward<Self>(self)));
			case xsd::DateTimeStamp::fixed_id.to_underlying():
				return f(get_specialized<xsd::DateTimeStamp>(std::forward<Self>(self)));
			case xsd::GYearMonth::fixed_id.to_underlying():
				return f(get_specialized<xsd::GYearMonth>(std::forward<Self>(self)));
			case xsd::Duration::fixed_id.to_underlying():
				return f(get_specialized<xsd::Duration>(std::forward<Self>(self)));
			case xsd::DayTimeDuration::fixed_id.to_underlying():
				return f(get_specialized<xsd::DayTimeDuration>(std::forward<Self>(self)));
			case xsd::YearMonthDuration::fixed_id.to_underlying():
				return f(get_specialized<xsd::YearMonthDuration>(std::forward<Self>(self)));
			default:
				assert(false);
				__builtin_unreachable();
		}
	}

	MetallNodeStorageBackendImpl::MetallNodeStorageBackendImpl(defs::allocator_type const &alloc)
		: bnode_storage_{alloc},
		  iri_storage_{alloc},
		  variable_storage_{alloc},
		  fallback_literal_storage_{alloc},
		  specialized_literal_storage_{std::allocator_arg, alloc} {

		// some iri's like xsd:string are there by default
		for (const auto &[iri, literal_type] : rdf4cpp::rdf::datatypes::registry::reserved_datatype_ids) {
			auto const id = literal_type.to_underlying();

			auto mem = iri_storage_.backend_allocator.allocate(1);
			new (std::to_address(mem)) MetallIRIBackend{MetallIRIBackend::View{iri}, alloc};

			auto [iter, inserted_successfully] = iri_storage_.data2id.emplace(mem, id);
			assert(inserted_successfully);
			iri_storage_.id2data.emplace(id, iter->first.get());
		}
	}

	size_t MetallNodeStorageBackendImpl::size() const noexcept {
		return iri_storage_.id2data.size() +
			   bnode_storage_.id2data.size() +
			   variable_storage_.id2data.size() +
			   fallback_literal_storage_.id2data.size() +
			   specialization_detail::tuple_fold(specialized_literal_storage_, size_t{0}, [](auto acc, auto const &storage) noexcept {
				   return acc + storage.id2data.size();
			   });
	}

	bool MetallNodeStorageBackendImpl::has_specialized_storage_for([[maybe_unused]] identifier::LiteralType datatype) const noexcept {
		static constexpr auto specialization_lut = specialization_detail::make_storage_specialization_lut<decltype(specialized_literal_storage_)>();
		return specialization_lut[datatype.to_underlying()];
	}

	/**
	 * Synchronized lookup (and creation) of IDs by a provided view of a Node Backend.
	 * @tparam create_if_not_present enables code for creating non-existing Node Backends
	 * @param view contains the data of the requested Node Backend
	 * @param storage the storage where the Node Backend is looked up
	 * @return the NodeID for the looked up Node Backend. Result is null() if there was no matching Node Backend.
	 */
	template<bool create_if_not_present, typename Storage>
	static identifier::NodeID lookup_or_insert_impl(typename Storage::BackendView const &view,
													Storage &storage) noexcept {

		{
			std::shared_lock lock{storage.mutex};
			if (auto const it = storage.data2id.find(view); it != storage.data2id.end()) {
				return it->second;
			}
		}

		if constexpr (!create_if_not_present) {
			return identifier::NodeID{};
		} else {
			std::unique_lock lock{storage.mutex};

			// check again, might have changed between unlocking of shared_lock and locking of unique_lock
			if (auto const it = storage.data2id.find(view); it != storage.data2id.end()) {
				return it->second;
			}

			identifier::NodeID const next_id = storage.id_gen.next_id(view);

			auto mem = storage.backend_allocator.allocate(1);
            new (std::to_address(mem)) typename Storage::Backend{view, storage.backend_allocator};

			auto [it, inserted] = storage.data2id.emplace(mem, next_id);
			assert(inserted);
			storage.id2data.emplace(next_id, it->first);

			return next_id;
		}
	}

	identifier::NodeID MetallNodeStorageBackendImpl::find_or_make_id(view::LiteralBackendView const &view) noexcept {
		return view.visit(
				[this](view::LexicalFormLiteralBackendView const &lexical) noexcept {
					assert(!this->has_specialized_storage_for(identifier::iri_node_id_to_literal_type(lexical.datatype_id)));

					return lookup_or_insert_impl<true>(lexical, fallback_literal_storage_);
				},
				[this](view::ValueLiteralBackendView const &any) noexcept -> identifier::NodeID {
					assert(this->has_specialized_storage_for(any.datatype));

					return visit_specialized(*this, any.datatype, [&](auto &storage) noexcept {
						return lookup_or_insert_impl<true>(any, storage);
					});
				});
	}

	identifier::NodeID MetallNodeStorageBackendImpl::find_or_make_id(view::IRIBackendView const &view) noexcept {
		return lookup_or_insert_impl<true>(view, iri_storage_);
	}

	identifier::NodeID MetallNodeStorageBackendImpl::find_or_make_id(view::BNodeBackendView const &view) noexcept {
		return lookup_or_insert_impl<true>(view, bnode_storage_);
	}

	identifier::NodeID MetallNodeStorageBackendImpl::find_or_make_id(view::VariableBackendView const &view) noexcept {
		return lookup_or_insert_impl<true>(view, variable_storage_);
	}

	identifier::NodeID MetallNodeStorageBackendImpl::find_id(view::BNodeBackendView const &view) const noexcept {
		return lookup_or_insert_impl<false>(view, bnode_storage_);
	}

	identifier::NodeID MetallNodeStorageBackendImpl::find_id(view::IRIBackendView const &view) const noexcept {
		return lookup_or_insert_impl<false>(view, iri_storage_);
	}

	identifier::NodeID MetallNodeStorageBackendImpl::find_id(view::LiteralBackendView const &view) const noexcept {
		return view.visit(
				[this](view::LexicalFormLiteralBackendView const &lexical) {
					assert(!this->has_specialized_storage_for(identifier::iri_node_id_to_literal_type(lexical.datatype_id)));

					return lookup_or_insert_impl<false>(lexical, fallback_literal_storage_);
				},
				[this](view::ValueLiteralBackendView const &any) -> identifier::NodeID {
					assert(this->has_specialized_storage_for(any.datatype));

					return visit_specialized(*this, any.datatype, [&any](auto const &storage) {
						return lookup_or_insert_impl<false>(any, storage);
					});
				});
	}

	identifier::NodeID MetallNodeStorageBackendImpl::find_id(view::VariableBackendView const &view) const noexcept {
		return lookup_or_insert_impl<false>(view, variable_storage_);
	}

	template<typename NodeTypeStorage>
	static typename NodeTypeStorage::BackendView find_backend_view(NodeTypeStorage &storage, identifier::NodeID const id) {
		std::shared_lock<std::shared_mutex> shared_lock{storage.mutex};
		return static_cast<typename NodeTypeStorage::BackendView>(*storage.id2data.at(id));
	}

	view::IRIBackendView MetallNodeStorageBackendImpl::find_iri_backend_view(identifier::NodeID const id) const {
		return find_backend_view(iri_storage_, id);
	}

	view::LiteralBackendView MetallNodeStorageBackendImpl::find_literal_backend_view(identifier::NodeID const id) const {
		if (auto const datatype = id.literal_type(); datatype.is_fixed() && has_specialized_storage_for(datatype)) {
			return visit_specialized(*this, datatype, [id](auto const &storage) {
				return find_backend_view(storage, id);
			});
		}

		return find_backend_view(fallback_literal_storage_, id);
	}

	view::BNodeBackendView MetallNodeStorageBackendImpl::find_bnode_backend_view(identifier::NodeID const id) const {
		return find_backend_view(bnode_storage_, id);
	}

	view::VariableBackendView MetallNodeStorageBackendImpl::find_variable_backend_view(identifier::NodeID const id) const {
		return find_backend_view(variable_storage_, id);
	}

	template<typename NodeTypeStorage>
	static bool erase_impl(NodeTypeStorage &storage, identifier::NodeID const id) noexcept {
		using Backend = typename NodeTypeStorage::Backend;
		using Backend_ptr = typename NodeTypeStorage::Backend_ptr;
		using BackendView = typename NodeTypeStorage::BackendView;

		std::unique_lock lock{storage.mutex};
		auto it = storage.id2data.find(id);
		if (it == storage.id2data.end()) {
			return false;
		}

		Backend_ptr backend_ptr = it->second;

		auto data_it = storage.data2id.find(static_cast<BackendView>(*backend_ptr));
		assert(data_it != storage.data2id.end());

		storage.id2data.erase(it);
		storage.data2id.erase(data_it);

		backend_ptr->~Backend();

		return true;
	}

	bool MetallNodeStorageBackendImpl::erase_iri(identifier::NodeID const id) noexcept {
		return erase_impl(iri_storage_, id);
	}

	bool MetallNodeStorageBackendImpl::erase_literal(identifier::NodeID const id) noexcept {
		if (id.literal_type().is_fixed() && this->has_specialized_storage_for(id.literal_type())) {
			return visit_specialized(*this, id.literal_type(), [id](auto &storage) noexcept {
				return erase_impl(storage, id);
			});
		}

		return erase_impl(fallback_literal_storage_, id);
	}

	bool MetallNodeStorageBackendImpl::erase_bnode(identifier::NodeID const id) noexcept {
		return erase_impl(bnode_storage_, id);
	}

	bool MetallNodeStorageBackendImpl::erase_variable(identifier::NodeID const id) noexcept {
		return erase_impl(variable_storage_, id);
	}

    bool MetallNodeStorageBackendImpl::is_in_memory() const noexcept {
		return bnode_storage_.backend_allocator.holds_allocator<defs::std_allocator_type_t>();
    }

    bool MetallNodeStorageBackendImpl::is_persistent() const noexcept {
		return bnode_storage_.backend_allocator.holds_allocator<defs::metall_allocator_type_t>();
    }

}// namespace dice::metall_node_storage