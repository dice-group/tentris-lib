#ifndef QUERY_UNIONOPERATOR_HPP
#define QUERY_UNIONOPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done>
	struct UnionOperator {

	private:
		inline static std::pair<std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>, std::vector<std::vector<size_t>>>
		init_union(std::vector<OperandDependencyGraph<htt_t, allocator_type>> &union_components,
				   std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				   Query<htt_t, allocator_type> const &q) {
			std::vector<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>> sub_operandss(union_components.size());
			std::vector<std::vector<size_t>> result_poss(union_components.size());
			for (size_t i = 0; i < union_components.size(); i++) {
				sub_operandss[i] = extract_operands(union_components[i], operands);
				for (auto const &label : union_components[i].operands_var_ids_set()) {
					if (q.is_var_tracked(label))
						result_poss[i].push_back(q.tracked_var_position(label));
				}
			}
			return std::make_pair(sub_operandss, result_poss);
		}

	public:
		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (not all_tracked_vars_done) {
			clear_used_entry_poss<htt_t, allocator_type>(entry_arg, odg, query);
			auto &union_comps = odg.union_components();
			auto [sub_operandss, result_poss] = init_union(union_comps, operands, query);
			for (size_t i = 0; i < union_comps.size(); i++) {
				evaluation_context.check_time_out();
				if (not query.all_tracked_vars_done(union_comps[i])) {
					co_yield std::elements_of(get_sub_operator<htt_t, allocator_type, false>(union_comps[i], sub_operandss[i], query, evaluation_context,entry_arg));
				} else {
					const auto &entry = get_sub_operator<htt_t, allocator_type, true>(union_comps[i], sub_operandss[i], query, evaluation_context, entry_arg);
					if (entry.value()) {
						co_yield entry_arg;
					}
				}
				clear_used_entry_poss<htt_t, allocator_type>(entry_arg, union_comps[i], query);
			}
		}

		inline static CountedEntry<htt_t> const &
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires (all_tracked_vars_done) {
			clear_used_entry_poss<htt_t, allocator_type>(entry_arg, odg, query);
			auto &union_comps = odg.union_components();
			auto [sub_operandss, result_poss] = init_union(union_comps, operands, query);
			unsigned long value = 0;
			for (size_t i = 0; i < union_comps.size(); i++) {
				evaluation_context.check_time_out();
				const auto &entry = get_sub_operator<htt_t, allocator_type, true>(union_comps[i], sub_operandss[i], query, evaluation_context, entry_arg);
				if (entry.value()) {
					value += entry.value();
				}
			}
			entry_arg.set_value(value);
			return entry_arg;
		}
	};
}// namespace dice::query::operators

#endif//QUERY_UNIONOPERATOR_HPP
