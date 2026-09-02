#ifndef QUERY_ENTRYGENERATOROPERATOR_HPP
#define QUERY_ENTRYGENERATOROPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done>
	struct EntryGeneratorOperator {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate([[maybe_unused]] OperandDependencyGraph<htt_t, allocator_type> const &odg,
				 [[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 [[maybe_unused]] Query<htt_t, allocator_type> const &query,
				 [[maybe_unused]] EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (not all_tracked_vars_done) {
			assert(false);
			// must not be used
			co_yield entry_arg;
		}

		inline static CountedEntry<htt_t> const &
		evaluate([[maybe_unused]] OperandDependencyGraph<htt_t, allocator_type> const &odg,
				 [[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 [[maybe_unused]] Query<htt_t, allocator_type> const &query,
				 [[maybe_unused]] EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (all_tracked_vars_done) {
			entry_arg.set_value(1);
			return entry_arg;
		}
	};
}// namespace dice::query::operators
#endif//QUERY_ENTRYGENERATOROPERATOR_HPP
