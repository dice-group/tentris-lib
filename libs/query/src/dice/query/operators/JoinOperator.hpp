#ifndef QUERY_JOINOPERATOR_HPP
#define QUERY_JOINOPERATOR_HPP

#include <dice/hypertrie/HashJoin.hpp>

#include <dice/query/operators/CardinalityEstimation.hpp>
#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done>
	struct JoinOperator {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (not all_tracked_vars_done) {
			clear_used_entry_poss<htt_t, allocator_type>(entry_arg, odg, query);
			char eval_var = CardinalityEstimation<htt_t, allocator_type>::getMinCardLabel(odg, operands, query);
			bool is_proj_var = query.is_var_tracked(eval_var);
			uint8_t proj_var_pos;
			if (is_proj_var)
				proj_var_pos = query.tracked_var_position(eval_var);
			auto &sub_odg = odg.remove_var_id(eval_var);
			auto sub_odg_all_result_done = query.all_tracked_vars_done(sub_odg);
			for (auto &[current_key_part, sub_operands] : hypertrie::HashJoin<htt_t, allocator_type>{operands,
																									 odg.var_ids_positions_in_operands(eval_var)}) {
				evaluation_context.check_time_out();
				if (is_proj_var)
					entry_arg[proj_var_pos] = current_key_part;
				if (sub_odg_all_result_done) {
					const auto &entry = get_sub_operator<htt_t, allocator_type, true>(sub_odg, sub_operands, query, evaluation_context, entry_arg);
					if (entry.value())
						co_yield entry;
				} else {
					co_yield std::elements_of(get_sub_operator<htt_t, allocator_type, false>(sub_odg, sub_operands, query, evaluation_context, entry_arg));
				}
			}
		}

		inline static CountedEntry<htt_t> const &
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (all_tracked_vars_done) {
			clear_used_entry_poss<htt_t, allocator_type>(entry_arg, odg, query);
			auto eval_var = CardinalityEstimation<htt_t, allocator_type>::getMinCardLabel(odg, operands, query);
			auto &sub_odg = odg.remove_var_id(eval_var);
			[[maybe_unused]] unsigned long value = 0;
			for (auto &[current_key_part, sub_operands] : hypertrie::HashJoin<htt_t, allocator_type>{operands,
																									 odg.var_ids_positions_in_operands(eval_var)}) {
				evaluation_context.check_time_out();
				const auto &entry = get_sub_operator<htt_t, allocator_type, true>(sub_odg, sub_operands, query, evaluation_context, entry_arg);
				value += entry.value();
			}
			entry_arg.set_value(value);
			return entry_arg;
		}
	};

}// namespace dice::query::operators
#endif//QUERY_JOINOPERATOR_HPP
