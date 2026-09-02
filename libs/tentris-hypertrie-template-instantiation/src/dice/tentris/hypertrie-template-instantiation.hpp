#ifndef TENTRIS_LIB_HYPERTRIE_TEMPLATE_INSTANTIATION_HPP
#define TENTRIS_LIB_HYPERTRIE_TEMPLATE_INSTANTIATION_HPP

#include <dice/tentris/param_allocator.hpp>
#include <dice/hash.hpp>
#include <dice/hypertrie.hpp>
#include <dice/node-wrapper.hpp>

namespace dice::tentris::defs {
	using key_part_type = dice::node_wrapper::NodeWrapper;
	template<typename Key, typename T, typename Hash, typename Equal, typename Allocator>
	using map_type = dice::sparse_map::sparse_map<Key,
												  T,
												  Hash,
												  Equal,
												  typename std::allocator_traits<Allocator>::template rebind_alloc<std::pair<Key, T>>,
												  dice::sparse_map::sh::power_of_two_growth_policy<2>,
												  dice::sparse_map::sh::exception_safety::basic,
												  dice::sparse_map::sh::sparsity::high>;

	template<typename Key, typename Hash, typename Equal, typename Allocator>
	using set_type = dice::sparse_map::sparse_set<
			Key,
			Hash,
			Equal,
			typename std::allocator_traits<Allocator>::template rebind_alloc<Key>,
			dice::sparse_map::sh::power_of_two_growth_policy<2>,
			dice::sparse_map::sh::exception_safety::basic,
			dice::sparse_map::sh::sparsity::high>;

	using htt_t = dice::hypertrie::Hypertrie_trait<key_part_type,
												   bool,
												   map_type,
												   set_type,
												   true>;
}// namespace dice::tentris::defs

namespace dice::hypertrie {
	extern template class HypertrieContext<tentris::defs::htt_t, tentris::defs::allocator_type>;

	extern template class Hypertrie<tentris::defs::htt_t, tentris::defs::allocator_type>;
	extern template class const_Hypertrie<tentris::defs::htt_t, tentris::defs::allocator_type>;

	extern template class Iterator<tentris::defs::htt_t, tentris::defs::allocator_type>;
	extern template class HashJoin<tentris::defs::htt_t, tentris::defs::allocator_type>;

	extern template class BulkUpdater<BulkUpdaterMode::Insert, tentris::defs::htt_t, tentris::defs::allocator_type, internal::raw::BulkUpdaterSyncness::Sync>;
	extern template class BulkUpdater<BulkUpdaterMode::Insert, tentris::defs::htt_t, tentris::defs::allocator_type, internal::raw::BulkUpdaterSyncness::Async>;

	extern template class BulkUpdater<BulkUpdaterMode::Remove, tentris::defs::htt_t, tentris::defs::allocator_type, internal::raw::BulkUpdaterSyncness::Sync>;
	extern template class BulkUpdater<BulkUpdaterMode::Remove, tentris::defs::htt_t, tentris::defs::allocator_type, internal::raw::BulkUpdaterSyncness::Async>;

	extern template class SliceKey<tentris::defs::htt_t>;
	extern template class Key<tentris::defs::htt_t>;
	extern template class NonZeroEntry<tentris::defs::htt_t>;
} // namespace dice::hypertrie

namespace dice::tentris::defs {
	using HypertrieContext = dice::hypertrie::HypertrieContext<htt_t, allocator_type>;
	using BoolHypertrie = dice::hypertrie::Hypertrie<htt_t, allocator_type>;
	using const_BoolHypertrie = dice::hypertrie::const_Hypertrie<htt_t, allocator_type>;
	using HypertrieSyncBulkInserter = dice::hypertrie::BulkInserter<htt_t, allocator_type, hypertrie::internal::raw::BulkUpdaterSyncness::Sync>;
	using HypertrieAsyncBulkInserter = dice::hypertrie::BulkInserter<htt_t, allocator_type, hypertrie::internal::raw::BulkUpdaterSyncness::Async>;
	using HypertrieSyncBulkRemover = dice::hypertrie::BulkRemover<htt_t, allocator_type, hypertrie::internal::raw::BulkUpdaterSyncness::Sync>;
	using HypertrieAsyncBulkRemover = dice::hypertrie::BulkRemover<htt_t, allocator_type, hypertrie::internal::raw::BulkUpdaterSyncness::Async>;
	using HypertrieContext_ptr = dice::hypertrie::HypertrieContext_ptr<htt_t, allocator_type>;
	using SliceKey = dice::hypertrie::SliceKey<htt_t>;
	using HTKey = dice::hypertrie::Key<htt_t>;
	using NonZeroEntry = dice::hypertrie::NonZeroEntry<htt_t>;
}// namespace dice::tentris::defs

namespace dice::hash {
template<typename Policy>
struct dice_hash_overload<Policy, rdf4cpp::rdf::Node> {
    static std::size_t dice_hash(rdf4cpp::rdf::Node const &x) noexcept {
        return Policy::hash_fundamental(x.backend_handle().raw());
    }
};

template<typename Policy>
struct dice_hash_overload<Policy, rdf4cpp::rdf::query::Variable> {
    static std::size_t dice_hash(rdf4cpp::rdf::query::Variable const &x) noexcept {
        return Policy::hash_fundamental(x.backend_handle().raw());
    }
};

template<typename Policy>
struct dice_hash_overload<Policy, rdf4cpp::rdf::Literal> {
    static std::size_t dice_hash(rdf4cpp::rdf::Literal const &x) noexcept {
        return Policy::hash_fundamental(x.backend_handle().raw());
    }
};

template<typename Policy>
struct dice_hash_overload<Policy, rdf4cpp::rdf::IRI> {
    static std::size_t dice_hash(rdf4cpp::rdf::IRI const &x) noexcept {
        return Policy::hash_fundamental(x.backend_handle().raw());
    }
};

template<typename Policy>
struct dice_hash_overload<Policy, rdf4cpp::rdf::BlankNode> {
    static std::size_t dice_hash(rdf4cpp::rdf::BlankNode const &x) noexcept {
        return Policy::hash_fundamental(x.backend_handle().raw());
    }
};
}  // namespace dice::hash

#endif//TENTRIS_LIB_HYPERTRIE_TEMPLATE_INSTANTIATION_HPP
