#ifndef QUERY_DISTINCTOPERATOR_HPP
#define QUERY_DISTINCTOPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>


namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct DistinctOperator {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg,
				 bool fully_pruned) {
			robin_hood::unordered_set<size_t, std::identity> found_entries{};
			for (auto const &entry : operators::get_project_operator<htt_t, allocator_type>(odg, operands, query, evaluation_context, entry_arg, fully_pruned)) {
				const size_t hash = dice::hash::DiceHashwyhash<CountedKey<htt_t>>()(entry.key());
				auto [_, is_new_key] = found_entries.emplace(hash);
				if (is_new_key) {
					// TODO: can we simply set entry.set_value(1) and return entry?
					// TODO: or could we have only one return_entry where we only change the key each time?
					co_yield CountedEntry<htt_t>{entry.key(), 1};
				}
			}
		}

	};

}// namespace dice::query::operators

#endif//QUERY_DISTINCTOPERATOR_HPP
