#ifndef QUERY_OPERATOR_HPP
#define QUERY_OPERATOR_HPP

#include <dice/query/operators/CartesianOperator.hpp>
#include <dice/query/operators/CountOperator.hpp>
#include <dice/query/operators/DistinctOperator.hpp>
#include <dice/query/operators/EntryGeneratorOperator.hpp>
#include <dice/query/operators/ExplicitAggregationOperator.hpp>
#include <dice/query/operators/FilterOperator.hpp>
#include <dice/query/operators/AssignmentOperator.hpp>
#include <dice/query/operators/ImplicitAggerationOperator.hpp>
#include <dice/query/operators/JoinOperator.hpp>
#include <dice/query/operators/LeftJoinOperator.hpp>
#include <dice/query/operators/LimitPaginationOperator.hpp>
#include <dice/query/operators/ProjectOperator.hpp>
#include <dice/query/operators/ResolveOperator.hpp>
#include <dice/query/operators/UnionOperator.hpp>

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline Operation next_op(OperandDependencyGraph<htt_t, allocator_type> &, Query<htt_t, allocator_type> const &);

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline std::generator<CountedEntry<htt_t> const &>
	get_project_operator(OperandDependencyGraph<htt_t, allocator_type> &odg,
						 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
						 Query<htt_t, allocator_type> const &query,
						 EvaluationContext<htt_t, allocator_type> &evaluation_context,
						 CountedEntry<htt_t> &entry,
						 bool fully_pruned) {
		if (query.contains_aggregates()) {
			if (not query.explicit_group())
				co_yield std::elements_of(operators::ImplicitAggregationOperator<htt_t, allocator_type>::evaluate(odg, operands, query, evaluation_context, entry));
			else if (not fully_pruned)
				co_yield std::elements_of(operators::ExplicitAggregationOperator<htt_t, allocator_type>::evaluate(odg, operands, query, evaluation_context, entry));
		} else if (not fully_pruned) {
			co_yield std::elements_of(operators::ProjectOperator<htt_t, allocator_type>::evaluate(odg, operands, query, evaluation_context, entry));
		}
	}

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done>
	inline std::conditional_t<all_tracked_vars_done, CountedEntry<htt_t> const &, std::generator<CountedEntry<htt_t> const &>>
	get_sub_operator(OperandDependencyGraph<htt_t, allocator_type> &odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					 Query<htt_t, allocator_type> const &query,
					 EvaluationContext<htt_t, allocator_type> &evaluation_context,
					 CountedEntry<htt_t> &entry) {
		switch (next_op(odg, query)) {
			case Operation::Join: {
				return JoinOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::LeftJoin: {
				return LeftJoinOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::Resolve: {
				return ResolveOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::Count: {
				return CountOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::Cartesian: {
				if (not odg.optional_cartesian())
					return CartesianOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
				return CartesianOperator<htt_t, allocator_type, all_tracked_vars_done, true>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::Union: {
				return UnionOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::Filter: {
				return FilterOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::Assignment: {
				return AssignmentOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::FilterAlt: {
				return FilterOperator<htt_t, allocator_type, all_tracked_vars_done, true>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			case Operation::EntryGenerator: {
				return EntryGeneratorOperator<htt_t, allocator_type, all_tracked_vars_done>::evaluate(odg, operands, query, evaluation_context, entry);
			}
			default:
				throw std::invalid_argument{"Undefined Operation"};
		}
	};

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline void clear_used_entry_poss(CountedEntry<htt_t> &entry,
									  OperandDependencyGraph<htt_t, allocator_type> &graph,
									  Query<htt_t, allocator_type> const &query) noexcept {
		for (auto const &result_pos : query.get_odg_tracked_vars_positions(graph)) {
			entry[result_pos] = {};
		}
	}

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline Operation next_op(OperandDependencyGraph<htt_t, allocator_type> &odg, Query<htt_t, allocator_type> const &query) {
		auto &operation_type = query.get_odg_operator_type(odg);
		if (operation_type != Operation::NoOp)
			return operation_type;
		if (odg.size() == 0) {// e.g. ->
			operation_type = Operation::EntryGenerator;
			return Operation::EntryGenerator;
		}
		if (odg.size() == 1) {// only one operand left
			if (odg.is_expression(0)) {
				if (odg.is_assignment(0)) {
					operation_type = Operation::Assignment;
					return Operation::Assignment;
				}
				operation_type = Operation::Filter;
				return Operation::Filter;
			}
			auto contains_proj_var = [&](char var) { return query.is_var_tracked(var); };
			if (odg.operands_var_ids_set().size() == odg.operand_var_ids(0).size()) {
				if (std::none_of(odg.operands_var_ids_set().begin(), odg.operands_var_ids_set().end(), contains_proj_var)) {// e.g. b->
					operation_type = Operation::Count;
					return Operation::Count;
				}
				if (std::all_of(odg.operands_var_ids_set().begin(), odg.operands_var_ids_set().end(), contains_proj_var)) {// e.g. a->a or aa->a
					operation_type = Operation::Resolve;
					return Operation::Resolve;
				}
			}
			// fallthrough, e.g. ab->a
		} else {                                    // more than one operand left
			if (odg.union_components().size() > 1) {// e.g. {a,b}{ab}->ab
				operation_type = Operation::Union;
				return Operation::Union;
			}
			if (not odg.assignments_for_evaluation().empty()) {
				operation_type = Operation::Assignment;
				return Operation::Assignment;
			}
			if (not odg.filters_for_evaluation().empty()) {
				operation_type = Operation::Filter;
				return Operation::Filter;
			}
			if (odg.cartesian_components().size() > 1) {// e.g. a,b->ab
				operation_type = Operation::Cartesian;
				return Operation::Cartesian;
			}
			if (odg.isc_operands().size() < odg.size()) {// e.g. a,[ab]->ab
				// if the ISC consists only of filters, we need to use the alternative filter operator
				// non_optional_var_ids_set() does not consider vertices corresponding to filters
				if (odg.non_optional_var_ids_set().empty()) {
					operation_type = Operation::FilterAlt;
					return Operation::FilterAlt;
				}
				// otherwise, LeftJoin
				operation_type = Operation::LeftJoin;
				return Operation::LeftJoin;
			}
			// fallthrough
		}
		operation_type = Operation::Join;
		return Operation::Join;
	}

	/*
	 * @brief Eliminates count components from cartesian products. The result of the counts is stored in the entry
	 */
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline std::optional<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>
	eliminate_counts(OperandDependencyGraph<htt_t, allocator_type> *&odg_ptr,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					 Query<htt_t, allocator_type> const &query,
					 unsigned long &counts_result) {
		std::vector<size_t> count_components_positions{};
		auto &cart_components = odg_ptr->cartesian_components();
		for (size_t i = 0; i < cart_components.size(); i++) {
			auto &cart_comp = cart_components[i];
			if (cart_comp.size() == 1 and query.all_tracked_vars_done(cart_comp))
				count_components_positions.push_back(i);
		}
		if (count_components_positions.empty())
			return std::nullopt;
		// compute the counts
		std::vector<uint8_t> vertices_to_remove{};
		std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> operands_after_elimination{};
		for (auto pos : count_components_positions) {
			auto operand_pos = cart_components[pos].operands_original_positions()[0];
			counts_result *= operands[pos].size();
			vertices_to_remove.push_back(operand_pos);
		}
		// update the graph
		odg_ptr = &(odg_ptr->remove_vertices(vertices_to_remove));
		// prepare the updated operands' vector
		for (size_t i = 0; i < operands.size(); i++) {
			if (std::find(vertices_to_remove.begin(), vertices_to_remove.end(), i) == vertices_to_remove.end())
				operands_after_elimination.push_back(operands[i]);
		}
		return operands_after_elimination;
	}

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline static std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>
	extract_operands(OperandDependencyGraph<htt_t, allocator_type> &odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands) {
		std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> sub_operands;
		for (const auto &original_op_pos : odg.operands_original_positions())
			sub_operands.emplace_back(operands[original_op_pos]);
		return sub_operands;
	}

}// namespace dice::query::operators

#endif//QUERY_OPERATOR_HPP
