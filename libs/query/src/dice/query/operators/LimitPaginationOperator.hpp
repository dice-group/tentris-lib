#ifndef QUERY_LIMITPAGINATIONOPERATOR_HPP
#define QUERY_LIMITPAGINATIONOPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>


namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct LimitPaginationOperator {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg,
				 bool fully_pruned) {
			std::generator<CountedEntry<htt_t> const &> sub_operator;
			if (query.is_distinct())
				sub_operator = operators::DistinctOperator<htt_t, allocator_type>::evaluate(odg, operands, query, evaluation_context, entry_arg, fully_pruned);
			else
				sub_operator = operators::get_project_operator<htt_t, allocator_type>(odg, operands, query, evaluation_context, entry_arg, fully_pruned);
			auto limit = query.limit();
			auto offset = query.offset();
			ssize_t limit_counter = 0;
			ssize_t offset_counter = 0;
			for (auto const &entry : sub_operator) {
				if (offset_counter < offset) {
					offset_counter += entry.value();
					if (offset_counter > offset) {
						count_type const value = offset_counter - offset;
						if (limit != -1 and value >= static_cast<count_type>(limit)) {
							co_yield CountedEntry<htt_t>{entry.key(), static_cast<count_type>(limit)};
							break;
						}
						limit_counter += value;
						co_yield CountedEntry<htt_t>{entry.key(), value};
					}
					continue;
				}
				if (limit != -1) {
					if (limit_counter + static_cast<ssize_t>(entry.value()) == limit) {
						co_yield entry;
						break;
					}
					if (limit_counter + static_cast<ssize_t>(entry.value()) > limit) {
						count_type const value = limit - limit_counter;
						co_yield CountedEntry<htt_t>{entry.key(), value};
						break;
					}
				}
				co_yield entry;
				limit_counter += entry.value();
			}
		}

	};

}// namespace dice::query::operators

#endif//QUERY_LIMITPAGINATIONOPERATOR_HPP
