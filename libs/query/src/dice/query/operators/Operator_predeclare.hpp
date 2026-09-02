#ifndef QUERY_OPERATOR_PREDECLARE_HPP
#define QUERY_OPERATOR_PREDECLARE_HPP

#include <dice/query/Commons.hpp>
#include <dice/query/OperandDependencyGraph.hpp>
#include <dice/query/Query.hpp>
#include <dice/query/util/generator.hpp>

#include <memory>
#include <utility>

namespace dice::query::operators {

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool all_result_done>
	inline std::conditional_t<all_result_done, CountedEntry<htt_t> const &, std::generator<CountedEntry<htt_t> const &>>
	get_sub_operator(OperandDependencyGraph<htt_t, allocator_type> &odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					 Query<htt_t, allocator_type> const &query,
					 EvaluationContext<htt_t, allocator_type> &evaluation_context,
					 CountedEntry<htt_t> &entry);

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline std::generator<CountedEntry<htt_t> const &>
	get_project_operator(OperandDependencyGraph<htt_t, allocator_type> &odg,
						 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
						 Query<htt_t, allocator_type> const &query,
						 EvaluationContext<htt_t, allocator_type> &evaluation_context,
						 CountedEntry<htt_t> &entry,
						 bool fully_pruned);

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline std::optional<std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>>
	eliminate_counts(OperandDependencyGraph<htt_t, allocator_type> *&odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands,
					 Query<htt_t, allocator_type> const &query,
					 unsigned long &counts_result);

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline void clear_used_entry_poss(CountedEntry<htt_t> &entry,
									  OperandDependencyGraph<htt_t, allocator_type> &graph,
									  Query<htt_t, allocator_type> const &query) noexcept;

	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	inline static std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>
	extract_operands(OperandDependencyGraph<htt_t, allocator_type> &odg,
					 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &operands);

	/**
	* @brief  Comparator for ordering solution sequences incrementally
	* @tparam htt_t The hypertie trait. Only boolean hypertries are supported
	*/
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct IncrementalSolutionOrdering {
	private:
		std::vector<ExpressionWrapper<htt_t, allocator_type>> expressions_;
		std::vector<bool> ordering_;

	public:
		IncrementalSolutionOrdering(std::vector<ExpressionWrapper<htt_t, allocator_type>> const &expressions, std::vector<bool> ordering)
			: expressions_(expressions), ordering_(std::move(ordering)) {}

		bool operator()(CountedEntry<htt_t> const &lhs, CountedEntry<htt_t> const &rhs) const {
			for (size_t i = 0; i < expressions_.size(); i++) {
				auto const &expr = expressions_[i];
				// evaluate expression using lhs
				expr.update_value(lhs.key());
				auto lhs_value = expr.evaluate();
				// evaluate expression using rhs
				expr.update_value(rhs.key());
				auto rhs_value = expr.evaluate();
				bool desc = ordering_[i];
				if (desc) {
					if (lhs_value > rhs_value) return true;
					else if (lhs_value < rhs_value) return false;
				} else {
					if (lhs_value < rhs_value) return true;
					else if (lhs_value > rhs_value) return false;
				}
			}
			return false;
		}
	};

	/**
	* @brief  Comparator for ordering grouped solution sequences
	* @tparam htt_t The hypertie trait. Only boolean hypertries are supported
	*/
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	struct GroupedSolutionOrdering {
	private:
		std::vector<bool> ordering_;

	public:
		GroupedSolutionOrdering(std::vector<bool> ordering)
			: ordering_(std::move(ordering)) {}

		bool operator()(std::pair<size_t, std::vector<ExpressionWrapper<htt_t, allocator_type>>> const &lhs,
						std::pair<size_t, std::vector<ExpressionWrapper<htt_t, allocator_type>>> const &rhs) const {
			auto const &lhs_expressions = lhs.second;
			auto const &rhs_expressions = rhs.second;
			for (size_t i = 0; i < lhs_expressions.size(); i++) {
				auto const &lhs_expr = lhs_expressions[i];
				auto const &rhs_expr = rhs_expressions[i];
				auto lhs_value = lhs_expr.evaluate();
				auto rhs_value = rhs_expr.evaluate();
				bool desc = ordering_[i];
				if (desc) {
					if (lhs_value > rhs_value) return true;
					else if (lhs_value < rhs_value) return false;
				} else {
					if (lhs_value < rhs_value) return true;
					else if (lhs_value > rhs_value) return false;
				}
			}
			return false;
		}
	};

}// namespace dice::query::operators

#endif//QUERY_OPERATOR_PREDECLARE_HPP
