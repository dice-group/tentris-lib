#ifndef QUERY_ASSIGNMENTOPERATOR_HPP
#define QUERY_ASSIGNMENTOPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done>
	struct AssignmentOperator {

		/*
		 * The assignment operator has two modes of operation.
		 * 1) In case the variable that is assigned a value is not resolved,
		 * the operator evaluates the expression of AssignmentExpressions and assigns the value to the variable.
		 * In this mode, the operator needs to slice the operands that use the assigned variable.
		 * If the slicing produces empty hypertries (or false), the resulting operand dependency graph is pruned.
		 * This might result in the operator to not return any solutions.
		 * 2) In case the variable that is assigned a value is already resolved (e.g., due to a join operation),
		 * the operator works as a filter. Here, the expression of AssignmentExpressions is evaluated and the operator
		 * checks whether the result of the expression equals the existing value of the variable. If the values are not equal,
		 * the operator does not produce any solutions.
		 * Use cases: BIND(), Optimization of FILTERs with equalities
		 */

		/**
		 * @brief Finds the empty operands and prunes the provided OperandDependencyGraph.
		 * @param odg: OperandDependencyGraph to be pruned
		 * @param operands: Vector of hypertries/value_types
		 * @return a pair consisting of the pruned graph and its corresponding operands
		 */
		inline static std::pair<OperandDependencyGraph<htt_t, allocator_type> *, std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>
		prune_empty(OperandDependencyGraph<htt_t, allocator_type> &odg,
					std::vector<std::variant<hypertrie::const_Hypertrie<htt_t, allocator_type>, typename htt_t::value_type>> const &sliced_operands) {
			std::vector<hypertrie::internal::pos_type> empty_sub_ops{};
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> pruned_operands{};
			for (size_t pos = 0; pos < sliced_operands.size(); pos++) {
				auto const &operand = sliced_operands[pos];
				if (std::holds_alternative<hypertrie::const_Hypertrie<htt_t, allocator_type>>(operand)) {
					if (std::get<0>(operand).empty())
						empty_sub_ops.push_back(pos);
				} else {
					if (not std::get<1>(operand))
						empty_sub_ops.push_back(pos);
				}
			}
			OperandDependencyGraph<htt_t, allocator_type> *pruned_odg = &odg.prune_graph(empty_sub_ops);
			for (auto pos : pruned_odg->operands_original_positions()) {
				if (std::holds_alternative<typename htt_t::value_type>(sliced_operands[pos]))
					continue; // always true here
				// keep scalars corresponding to expressions
				if (odg.is_expression(pos) or std::get<0>(sliced_operands[pos]).depth() > 0  )
					pruned_operands.push_back(std::get<0>(sliced_operands[pos]));
			}
			return std::make_pair(pruned_odg, pruned_operands);
		}

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires(not all_tracked_vars_done)  {
			auto eval_impl_result = evaluate_impl(odg, operands, query, evaluation_context, entry_arg);
			if (not eval_impl_result.has_value())
				co_return;
			auto [pruned_odg, pruned_sub_operands] = eval_impl_result.value();
			// run suboperator
			auto sub_odg_all_result_done = query.all_tracked_vars_done(*pruned_odg);
			if (sub_odg_all_result_done) {
				const auto &entry = get_sub_operator<htt_t, allocator_type, true>(*pruned_odg, pruned_sub_operands,
																				  query, evaluation_context, entry_arg);
				if (entry.value())
					co_yield entry;
			} else {
				co_yield std::elements_of(get_sub_operator<htt_t, allocator_type, false>(*pruned_odg, pruned_sub_operands,
																						 query, evaluation_context, entry_arg));
			}
		}

		inline static CountedEntry<htt_t> const &
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires(all_tracked_vars_done) {
			auto eval_impl_result = evaluate_impl(odg, operands, query, evaluation_context, entry_arg);
			if (not eval_impl_result.has_value()) {
				entry_arg.set_value(0);
				return entry_arg;
			}
			auto [pruned_odg, pruned_sub_operands] = eval_impl_result.value();
			return get_sub_operator<htt_t, allocator_type, true>(*pruned_odg, pruned_sub_operands,
																 query, evaluation_context, entry_arg);
		}

	private:
		static std::optional<std::pair<OperandDependencyGraph<htt_t, allocator_type> *, std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>>
		evaluate_impl(OperandDependencyGraph<htt_t, allocator_type> &odg,
					  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					  Query<htt_t, allocator_type> const &query,
					  EvaluationContext<htt_t, allocator_type> &evaluation_context,
					  CountedEntry<htt_t> &entry_arg) {
			auto const &assignments = odg.assignments_for_evaluation();
			bool has_result = true;
			boost::container::flat_map<char, typename htt_t::key_part_type> vars_to_resolve{};
			for (auto assignment_pos : assignments) {
				auto assignment = odg.get_assignment(assignment_pos);
				assignment.update_value(entry_arg.key());
				auto expr_result = assignment.evaluate(evaluation_context);
				auto result_var_id = assignment.result_var_id();
				assert(query.tracked_vars_positions().contains(result_var_id));
				// check if it result_var_id has already been resolved
				if (odg.removed_var_ids().contains(result_var_id)) {
					// check if the result of the expression matches the existing value
					if (expr_result != entry_arg[query.tracked_var_position(result_var_id)]) {
						has_result = false;
						break;
					}
				} else {
					// updated the entry with the value of the expression
					entry_arg[query.tracked_var_position(result_var_id)] = expr_result;
					// keep track of the variables that were resolved in this recursive step
					vars_to_resolve[result_var_id] = expr_result;
				}
			}
			// return if the result of one of the expressions does not match the existing value of the variable
			if (not has_result)
				return std::nullopt;
			// slicing and preparation of the next operator starts here
			// prepare the slice keys
			std::vector<hypertrie::SliceKey<htt_t>> slice_keys{};
			for (size_t i = 0; i < operands.size(); i++) {
				slice_keys.push_back(hypertrie::SliceKey<htt_t>::make_unbound(operands[i].depth()));
			}
			for (auto const &[var_id, value] : vars_to_resolve) {
				// slice only if the variable appears in the operands
				if (not odg.operands_var_ids_set().contains(var_id))
					continue;
				auto const &var_ids_in_operands = odg.var_ids_positions_in_operands(var_id);
				for (size_t i = 0; i < operands.size(); i++) {
					for (auto pos : var_ids_in_operands[i]) {
						slice_keys[i][pos] = value;
					}
				}
			}
			std::vector<std::variant<hypertrie::const_Hypertrie<htt_t, allocator_type>, typename htt_t::value_type>> sliced_operands{};
			for (size_t i = 0; i < operands.size(); i++) {
				// skip the operands of the assignments as they will be removed (currently true rank-0 tensors)
				if (std::find(assignments.begin(), assignments.end(), i) != assignments.end())
					continue;
				if (operands[i].depth() > 0)// do not slice operands of expressions
					sliced_operands.push_back(operands[i][slice_keys[i]]);
				else
					sliced_operands.push_back(operands[i]);
			}
			// remove the vertices of the assignments that were resolved in this recursive step
			OperandDependencyGraph<htt_t, allocator_type> *sub_odg = &odg.remove_vertices(assignments);
			if (sub_odg->size() == 0)
				return std::make_pair(sub_odg, std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>{});
			// prune empty operands
			auto [pruned_odg, pruned_sub_operands] = prune_empty(*sub_odg, sliced_operands);
			for (auto const &var_to_resolve : vars_to_resolve) {
				pruned_odg = &(pruned_odg->remove_var_id(var_to_resolve.first));
			}
			// the graph was completely pruned, hence no results should be returned
			if (pruned_odg->size() == 0 and sub_odg->size() > 0)
				return std::nullopt;
			return std::make_pair(pruned_odg, std::move(pruned_sub_operands));
		}
	};
}// namespace dice::query::operators

#endif//QUERY_ASSIGNMENTOPERATOR_HPP