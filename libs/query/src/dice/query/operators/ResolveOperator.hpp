#ifndef QUERY_RESOLVEOPERATOR_HPP
#define QUERY_RESOLVEOPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done>
	struct ResolveOperator {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 [[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (not all_tracked_vars_done) {
			assert(operands.size() == 1);// only one operand must be left to be resolved
			assert(not operands[0].empty());
			clear_used_entry_poss<htt_t, allocator_type>(entry_arg, odg, query);
			auto const &operand_vars = odg.operand_var_ids(0);
			// save the positions of the operand's variables
			std::vector<size_t> vars_positions{};
			vars_positions.reserve(operand_vars.size());
			for (auto const &op_var : operand_vars) {
				vars_positions.push_back(query.tracked_var_position(op_var));
			}
			for (const auto &operand_entry : operands[0]) {
				evaluation_context.check_time_out();
				for (size_t i = 0; i < operand_entry.size(); ++i) {
					entry_arg[vars_positions[i]] = operand_entry[i];
				}
				entry_arg.set_value(1);
				co_yield entry_arg;
			}
		}

		inline static CountedEntry<htt_t> const &
		evaluate([[maybe_unused]] OperandDependencyGraph<htt_t, allocator_type> &odg,
				 [[maybe_unused]] std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 [[maybe_unused]] Query<htt_t, allocator_type> const &query,
				 [[maybe_unused]] EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (all_tracked_vars_done) {
			assert(false);// should never be scheduled
			return entry_arg;
		}
	};

}// namespace dice::query::operators
#endif//QUERY_RESOLVEOPERATOR_HPP
