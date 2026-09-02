#ifndef QUERY_EXPLICITAGGREGATIONOPERATOR_HPP
#define QUERY_EXPLICITAGGREGATIONOPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct ExplicitAggregationOperator {

		enum FilteringModifier { FilteringRequired = true, FilteringNotRequired = false };
		enum OrderingModifier { OrderingRequired = true, OrderingNotRequired = false };

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) {
			bool has_filters = not query.solution_filters().empty();
			bool has_ordering = not query.ordering_type().empty();
			if (has_ordering and has_filters)
				co_yield std::elements_of(evaluate_impl<FilteringModifier::FilteringRequired, OrderingModifier::OrderingRequired>(odg, operands, query, evaluation_context, entry_arg));
			else if (has_ordering)
				co_yield std::elements_of(evaluate_impl<FilteringModifier::FilteringNotRequired, OrderingModifier::OrderingRequired>(odg, operands, query, evaluation_context, entry_arg));
			else if (has_filters)
				co_yield std::elements_of(evaluate_impl<FilteringModifier::FilteringRequired, OrderingModifier::OrderingNotRequired>(odg, operands, query, evaluation_context, entry_arg));
			else
				co_yield std::elements_of(evaluate_impl<FilteringModifier::FilteringNotRequired, OrderingModifier::OrderingNotRequired>(odg, operands, query, evaluation_context, entry_arg));
		}

	private:
		template<FilteringModifier FModifier, OrderingModifier OModifier>
		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate_impl(OperandDependencyGraph<htt_t, allocator_type> &odg,
					  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					  Query<htt_t, allocator_type> const &query,
					  EvaluationContext<htt_t, allocator_type> &evaluation_context,
					  CountedEntry<htt_t> &entry_arg) {
			auto const &solution_expressions = query.solution_mapping();
			auto const &solution_filters = query.solution_filters();
			auto const &solution_ordering = query.order_by_expressions();
			auto const &grouping_key_expressions = query.grouping_key();
			auto solution_entry = CountedEntry<htt_t>::make_with_defaulted_key(solution_expressions.size(), 1);
			auto grouping_key = CountedKey<htt_t>::make_defaulted(grouping_key_expressions.size());
			robin_hood::unordered_flat_map<size_t, std::vector<ExpressionWrapper<htt_t, allocator_type>>> grouped_solutions{};
			robin_hood::unordered_flat_map<size_t, std::vector<ExpressionWrapper<htt_t, allocator_type>>> grouped_filters{};
			robin_hood::unordered_flat_map<size_t, std::vector<ExpressionWrapper<htt_t, allocator_type>>> grouped_orderings{};
			for (auto const &entry : operators::get_sub_operator<htt_t, allocator_type, false>(odg, operands, query, evaluation_context, entry_arg)) {
				// compute the key for the solution
				for (size_t i = 0; i < grouping_key_expressions.size(); i++) {
					grouping_key_expressions[i].update_value(entry.key());
					grouping_key[i] = grouping_key_expressions[i].evaluate();
				}
				size_t gk_hash = dice::hash::DiceHashwyhash<CountedKey<htt_t>>()(grouping_key);
				// group_key_lookup can be evaluated using only a single map (if it is missing from one of the maps then it is missing from all of them)
				auto group_key_lookup = grouped_solutions.find(gk_hash);
				auto found = group_key_lookup != grouped_solutions.end();
				auto &grouped_solution = found ? group_key_lookup->second
											   : grouped_solutions.emplace(gk_hash, solution_expressions).first->second;
				// update the values of aggregate expressions
				auto cardinality = entry.value();
				while (cardinality > 0) {
					for (auto &expr : grouped_solution) {
						expr.update_value(entry.key());
					}
					cardinality--;
				}
				// update the values of solution filters
				if constexpr (FModifier) {
					cardinality = entry.value();
					auto &grouped_filter = found ? grouped_filters[gk_hash]
												 : grouped_filters.emplace(gk_hash, solution_filters).first->second;
					while (cardinality > 0) {
						for (auto &expr : grouped_filter) {
							expr.update_value(entry.key());
						}
						cardinality--;
					}
				}
				// update the values of ordering expressions
				if constexpr (OModifier) {
					cardinality = entry.value();
					auto &grouped_ordering = found ? grouped_orderings[gk_hash]
												   : grouped_orderings.emplace(gk_hash, solution_ordering).first->second;
					while (cardinality > 0) {
						for (auto &expr : grouped_ordering) {
							expr.update_value(entry.key());
						}
						cardinality--;
					}
				}
			}
			if constexpr (FModifier)
				assert(grouped_solutions.size() == grouped_filters.size());
			if constexpr (OModifier)
				assert(grouped_solutions.size() == grouped_orderings.size());
			if constexpr (not OModifier) {
				for (auto const &[key, group_solution] : grouped_solutions) {
					if constexpr (FModifier) {
						bool filters_satisfied = true;
						auto const &grouped_filter = grouped_filters[key];
						for (auto &expr : grouped_filter) {
							auto expr_res = expr.evaluate();
							if (not expr_res) {
								filters_satisfied = false;
								break;
							}
						}
						if (not filters_satisfied)
							continue;
					}
					for (size_t i = 0; i < group_solution.size(); i++) {
						solution_entry[i] = group_solution[i].evaluate();
					}
					co_yield solution_entry;
				}
			} else {
				// use multiset to sort entries. set is not suitable, as it does not insert elements that are not equal
				// but share the same values in the positions that are used for ordering.
				// Example: <1,2> and <2,2> are not the same but if only the second position is used for ordering, then they will be considered equal.
				boost::container::multiset<std::pair<size_t, std::vector<ExpressionWrapper<htt_t, allocator_type>>>,
							  GroupedSolutionOrdering<htt_t, allocator_type>> ordered_group_keys(GroupedSolutionOrdering<htt_t, allocator_type>(query.ordering_type()));
				for (auto const &[key, group_solution] : grouped_solutions) {
					if constexpr (FModifier) {
						bool filters_satisfied = true;
						auto const &grouped_filter = grouped_filters[key];
						for (auto &expr : grouped_filter) {
							auto expr_res = expr.evaluate();
							if (not expr_res) {
								filters_satisfied = false;
								break;
							}
						}
						if (not filters_satisfied)
							continue;
					}
					// order the keys that satisfy the filters
					ordered_group_keys.emplace(key, grouped_orderings[key]);
				}
				for (auto const &pair : ordered_group_keys) {
					auto const &group_solution = grouped_solutions[pair.first];
					for (size_t i = 0; i < group_solution.size(); i++) {
						solution_entry[i] = group_solution[i].evaluate();
					}
					co_yield solution_entry;
				}
			}
		}
	};

}// namespace dice::query::operators

#endif//QUERY_EXPLICITAGGREGATIONOPERATOR_HPP
