#ifndef QUERY_COUNTOPERATOR_HPP
#define QUERY_COUNTOPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done>
	struct CountOperator {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate([[maybe_unused]] OperandDependencyGraph<htt_t, allocator_type> &odg,
				 [[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 [[maybe_unused]] Query<htt_t, allocator_type> const &query,
				 [[maybe_unused]] EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (not all_tracked_vars_done) {
			assert(false);// only one operand must be left to be resolved
			co_yield entry_arg;
		}

		inline static CountedEntry<htt_t> const &
		evaluate([[maybe_unused]] OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 [[maybe_unused]] Query<htt_t, allocator_type> const &query,
				 [[maybe_unused]] EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (all_tracked_vars_done) {
			assert(operands.size() == 1);// only one operand must be left to be resolved
			assert(not operands[0].empty());
			entry_arg.set_value(operands[0].size());
			return entry_arg;
		}
	};

}// namespace dice::query::operators
#endif//QUERY_COUNTOPERATOR_HPP
