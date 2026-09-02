#include <dice/tentris/hypertrie-template-instantiation.hpp>

namespace dice::hypertrie {
	template class HypertrieContext<tentris::defs::htt_t, tentris::defs::allocator_type>;

	template class Hypertrie<tentris::defs::htt_t, tentris::defs::allocator_type>;
	template class const_Hypertrie<tentris::defs::htt_t, tentris::defs::allocator_type>;

	template class Iterator<tentris::defs::htt_t, tentris::defs::allocator_type>;
	template class HashJoin<tentris::defs::htt_t, tentris::defs::allocator_type>;

	template class BulkUpdater<BulkUpdaterMode::Insert, tentris::defs::htt_t, tentris::defs::allocator_type, internal::raw::BulkUpdaterSyncness::Sync>;
	template class BulkUpdater<BulkUpdaterMode::Insert, tentris::defs::htt_t, tentris::defs::allocator_type, internal::raw::BulkUpdaterSyncness::Async>;

	template class BulkUpdater<BulkUpdaterMode::Remove, tentris::defs::htt_t, tentris::defs::allocator_type, internal::raw::BulkUpdaterSyncness::Sync>;
	template class BulkUpdater<BulkUpdaterMode::Remove, tentris::defs::htt_t, tentris::defs::allocator_type, internal::raw::BulkUpdaterSyncness::Async>;

	template class SliceKey<tentris::defs::htt_t>;
	template class Key<tentris::defs::htt_t>;
	template class NonZeroEntry<tentris::defs::htt_t>;
} // dice::hypertrie
