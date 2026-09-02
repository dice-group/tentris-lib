#ifndef QUERY_PROJECTOPERATOR_HPP
#define QUERY_PROJECTOPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>


namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct ProjectOperator {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) {
			auto solution_mapping = query.solution_mapping();
			auto solution_entry = CountedEntry<htt_t>::make_with_defaulted_key(solution_mapping.size(), 1);
			// no tracking variables -> no projected variables
			if (query.all_tracked_vars_done(odg)) {
				// evaluate expressions in projection
				auto entry = operators::get_sub_operator<htt_t, allocator_type, true>(odg, operands, query, evaluation_context, entry_arg);
				for (size_t i = 0; i < solution_mapping.size(); i++) {
					solution_mapping[i].update_value(entry.key());
					solution_entry[i] = solution_mapping[i].evaluate();
				}
				solution_entry.set_value(entry.value());
				co_yield solution_entry;
			}
			// unordered solution sequence
			else if (query.ordering_type().empty()) {
				for (auto const &entry : operators::get_sub_operator<htt_t, allocator_type, false>(odg, operands, query, evaluation_context, entry_arg)) {
					for (size_t i = 0; i < solution_mapping.size(); i++) {
						solution_mapping[i].update_value(entry.key());
						solution_entry[i] = solution_mapping[i].evaluate();
					}
					auto val = entry.value();
					solution_entry.set_value(val);
					co_yield solution_entry;
				}
			}
			// ordered solution sequence
			else {
				// use multiset to sort entries. a map is not suitable, as it overwrites keys that are not equal
				// but share the same values in the positions that are used for ordering.
				// Example: <1,2> and <2,2> are not the same but if only the second position is used for ordering, then they will be considered equal.
				boost::container::multiset<CountedEntry<htt_t>, IncrementalSolutionOrdering<htt_t, allocator_type>>
						sorted_entries(IncrementalSolutionOrdering(query.order_by_expressions(), query.ordering_type()));
				// sort entries
				for (auto const &entry : operators::get_sub_operator<htt_t, allocator_type, false>(odg, operands, query, evaluation_context, entry_arg)) {
					sorted_entries.insert(entry);
				}
				// project using the sorted ordering
				for (auto const &entry : sorted_entries) {
					for (size_t i = 0; i < solution_mapping.size(); i++) {
						solution_mapping[i].update_value(entry.key());
						solution_entry[i] = solution_mapping[i].evaluate();
					}
					solution_entry.set_value(entry.value());
					co_yield solution_entry;
				}
			}
		}
	};

}// namespace dice::query::operators

#endif//QUERY_PROJECTOPERATOR_HPP
