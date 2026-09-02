#ifndef QUERY_FILTEROPERATOR_HPP
#define QUERY_FILTEROPERATOR_HPP

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done, bool Optional = false>
	struct FilterOperator {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires(not all_tracked_vars_done) {
			auto const &filters = odg.filters_for_evaluation();
			bool result = true;
			for (auto filter_pos : filters) {
				auto filter = odg.get_filter(filter_pos);
				filter.update_value(entry_arg.key());
				result &= filter.evaluate(evaluation_context);
				if (not result) break;
			}
			if (not result)
				co_return;
			auto &sub_odg = odg.remove_vertices(filters);
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> sub_operands{};
			for (auto orig_pos : sub_odg.operands_original_positions()) {
				sub_operands.push_back(operands[orig_pos]);
			}
			co_yield std::elements_of(get_sub_operator<htt_t, allocator_type, false>(sub_odg, sub_operands, query, evaluation_context, entry_arg));
		}

		inline static CountedEntry<htt_t> const &
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires(all_tracked_vars_done) {
			auto const &filters = odg.filters_for_evaluation();
			bool result = true;
			for (auto filter_pos : filters) {
				auto filter = odg.get_filter(filter_pos);
				filter.update_value(entry_arg.key());
				result &= filter.evaluate(evaluation_context);
				if (not result) break;
			}
			if (not result) {
				entry_arg.set_value(0);
				return entry_arg;
			}
			auto &sub_odg = odg.remove_vertices(filters);
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> sub_operands{};
			for (auto orig_pos : sub_odg.operands_original_positions()) {
				sub_operands.push_back(operands[orig_pos]);
			}
			const auto &entry = get_sub_operator<htt_t, allocator_type, true>(sub_odg, sub_operands, query, evaluation_context, entry_arg);
			return entry;
		}
	};

	/**
	 * @brief Filter operator used for filters whose variables appear only in optional parts of the query.
	 * Acts as a barrier for the results generated from the optional part of the query.
	 * @tparam htt_t boolean hypertrie trait
	 * @tparam allocator_type the type of the allocator
	 */
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_tracked_vars_done>
	struct FilterOperator<htt_t, allocator_type, all_tracked_vars_done, true> {

		inline static std::generator<CountedEntry<htt_t> const &>
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				Query<htt_t, allocator_type> const &query,
				EvaluationContext<htt_t, allocator_type> &evaluation_context,
				CountedEntry<htt_t> &entry_arg) requires(not all_tracked_vars_done) {
			auto const &filters = odg.isc_operands();
			auto &sub_odg = odg.remove_vertices(filters);
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> sub_operands{};
			for (auto orig_pos : sub_odg.operands_original_positions()) {
				sub_operands.push_back(operands[orig_pos]);
			}
			bool at_least_one_result = false;
			for (auto const &entry : get_sub_operator<htt_t, allocator_type, false>(sub_odg, sub_operands, query, evaluation_context, entry_arg)) {
				at_least_one_result = true;
				bool result = true;
				for (auto filter_pos : filters) {
					auto filter = odg.get_filter(filter_pos);
					filter.update_value(entry.key());
					result &= filter.evaluate(evaluation_context);
					if (not result) break;
				}
				if (result)
					co_yield entry;
			}
			// if the sub_operator did not yield any results check whether the current entry satisfies the filter
			// Note: the optional part must return at least one solution
			if (not at_least_one_result) {
				// clear the key_parts that might have been assigned by the sub_operator before failing
				clear_used_entry_poss(entry_arg, sub_odg, query);
				bool result = true;
				for (auto filter_pos : filters) {
					auto filter = odg.get_filter(filter_pos);
					filter.update_value(entry_arg.key());
					result &= filter.evaluate(evaluation_context);
					if (not result) break;
				}
				if (result) {
					entry_arg.set_value(1);
					co_yield entry_arg;
				}
			}
		}

		inline static CountedEntry<htt_t> const &
		evaluate(OperandDependencyGraph<htt_t, allocator_type> &odg,
				 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
				 Query<htt_t, allocator_type> const &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context,
				 CountedEntry<htt_t> &entry_arg) requires(all_tracked_vars_done) {
			auto const &filters = odg.isc_operands();
			bool result = true;
			auto &sub_odg = odg.remove_vertices(filters);
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> sub_operands{};
			for (auto orig_pos : sub_odg.operands_original_positions()) {
				sub_operands.push_back(operands[orig_pos]);
			}
			const auto &entry = get_sub_operator<htt_t, allocator_type, true>(sub_odg, sub_operands, query, evaluation_context, entry_arg);
			for (auto filter_pos : filters) {
				auto filter = odg.get_filter(filter_pos);
				filter.update_value(entry.key());
				result &= filter.evaluate(evaluation_context);
				if (not result) break;
			}
			if (not result) {
				entry_arg.set_value(0);
			}
			return entry;
		}
	};

}// namespace dice::query::operators

#endif//QUERY_FILTEROPERATOR_HPP
