#ifndef QUERY_EVALUATION_HPP
#define QUERY_EVALUATION_HPP

#include <memory>
#include <utility>

#include <boost/container/set.hpp>

#include <dice/query/Commons.hpp>
#include <dice/query/OperandDependencyGraph.hpp>
#include <dice/query/Query.hpp>
#include <dice/query/operators/Operator.hpp>
#include <dice/query/util/generator.hpp>

namespace dice::query {

	class Evaluation {
	public:
		/**
		 * @brief Entry point for the evaluation of queries.
		 * <p> It first removes operands that are empty along with their dependent operands. </p>
		 * <p> It then removes operands that are scalars, whose value is true. Such operands do not affect the evaluation. </p>
		 * <p> Scalars corresponding to filters are not removed </p>
		 * <p> It is responsible for calling the appropriate eval function. </p>
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param query The query object
		 * @param end_time The timeout
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static std::generator<CountedEntry<htt_t> const &>
		evaluate(Query<htt_t, allocator_type> &query,
				 EvaluationContext<htt_t, allocator_type> &evaluation_context) {
			auto entry_arg = CountedEntry<htt_t>::make_with_defaulted_key(query.tracked_vars().size(), 1);
			// from now on we are going to be working with processed graph
			// this allows to update the caches of the query's graph, without overwriting it.
			OperandDependencyGraph<htt_t, allocator_type> *processed_graph = &query.operand_dependency_graph();
			// no operands (in SPARQL empty WHERE clause -> return a single solution)
			bool already_empty = processed_graph->size() == 0;
			// check if any of the variables are already resolved (update slice keys for resolved variables)
			resolve_variables(query, processed_graph, entry_arg);
			auto query_operands = generate_query_operands(query, processed_graph,evaluation_context);
			auto pruned_ops = prune_empty_operands(processed_graph, query_operands);
			bool fully_pruned = processed_graph->size() == 0 && not already_empty;
			auto finalized_ops = remove_rank0_operands(processed_graph, pruned_ops);
			if (query.limit() > -1 or query.offset() > -1)
				co_yield std::elements_of(operators::LimitPaginationOperator<htt_t, allocator_type>::evaluate(*processed_graph, finalized_ops, query, evaluation_context, entry_arg, fully_pruned));
			else if (query.is_distinct())
				co_yield std::elements_of(operators::DistinctOperator<htt_t, allocator_type>::evaluate(*processed_graph, finalized_ops, query, evaluation_context, entry_arg, fully_pruned));
			else
				co_yield std::elements_of(operators::get_project_operator<htt_t, allocator_type>(*processed_graph, finalized_ops, query, evaluation_context, entry_arg, fully_pruned));
		}

		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static hypertrie::Hypertrie<htt_t, allocator_type>
		evaluate_as_hypertrie(Query<htt_t, allocator_type> &query,
							  EvaluationContext<htt_t, allocator_type> &evaluation_context) {
			auto depth = query.solution_mapping().size();
			assert(depth <= 3);
			hypertrie::Hypertrie<htt_t, allocator_type> result{depth, query.tensor_context().get()};
			hypertrie::BulkInserter<htt_t, allocator_type, hypertrie::internal::raw::BulkUpdaterSyncness::Sync> bulk_inserter{result};
			auto boolean_entry = hypertrie::NonZeroEntry<htt_t>::make_with_defaulted_key(depth, 1);
			for (auto const &entry : evaluate(query, evaluation_context)) {
				std::copy(entry.key().begin(), entry.key().end(), boolean_entry.key().begin());
				bulk_inserter.add(boolean_entry);
			}
			bulk_inserter.flush();
			return result;
		}

	private:

		/**
		 * @brief Generates the operands of the query. Uses the slice keys stored in the operand dependency graph.
		 * Subqueries are also evaluated.
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>
		generate_query_operands(Query<htt_t, allocator_type> &query,
								OperandDependencyGraph<htt_t, allocator_type> *&odg_ptr,
								EvaluationContext<htt_t, allocator_type> &evaluation_context) {
			// SAFETY of storing const_Hypertrie here:
			// Operands are:
			// 		- slices of evaluation_context.tensor => references the triplestore and is thus guaranteed to live at least until the end of query eval
			// 		- subquery operands which are stored in query and then referenced => trivially live until the end of query eval
			//		- the true scalar => even a const_Hypertrie scalar does not depend on the lifetime of anything else
			//
			// In all cases they are guaranteed to live at least until query is evaluated
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> query_operands{};

			// evaluate subqueries first
			for (auto &[id, subquery] : query.subqueries()) {
				query.add_subquery_operand(id, evaluate_as_hypertrie(subquery, evaluation_context));
			}
			query_operands.resize(odg_ptr->size());
			for (size_t i = 0; i < odg_ptr->size(); i++) {
				if (odg_ptr->is_expression(i)) {
					query_operands[i] = hypertrie::const_Hypertrie<htt_t, allocator_type>::from_scalar(true);
				} else {
					if (auto slice_key_ptr = odg_ptr->get_slice_key(i); slice_key_ptr != nullptr) {
						auto slice_result = evaluation_context.tensor[*slice_key_ptr];
						query_operands[i] = slice_result;
					} else if (auto subquery_id = odg_ptr->get_subquery_id(i); subquery_id.has_value()) {
						query_operands[i] = query.subquery_operand(subquery_id.value());
					} else if (auto inline_data_id = odg_ptr->get_inline_data_id(i); inline_data_id.has_value()) {
						query_operands[i] = query.inline_data(inline_data_id.value());
					} else {
						assert(false);
					}
				}
			}
			return query_operands;
		}

		/**
		 * @brief Removes operands that are empty and their dependent operands.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param odg_ptr
		 * @param ops
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>
		prune_empty_operands(OperandDependencyGraph<htt_t, allocator_type> *&odg_ptr,
							 std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &ops) {
			std::vector<uint8_t> empty_ops{};
			for (uint8_t i = 0; i < ops.size(); i++) {
				if (ops[i].empty())
					empty_ops.push_back(i);
			}
			if (empty_ops.empty())
				return ops;
			odg_ptr = &(odg_ptr->prune_graph(empty_ops));
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> new_ops{};
			for (auto pos : odg_ptr->operands_original_positions()) {
				new_ops.push_back(ops[pos]);
			}
			return new_ops;
		}

		/**
		 * @brief Removes scalar operands.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param odg_ptr
		 * @param ops
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>>
		remove_rank0_operands(OperandDependencyGraph<htt_t, allocator_type> *&odg_ptr,
							  std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> const &ops) {
			std::vector<uint8_t> rank0_ops{};
			for (size_t i = 0; i < ops.size(); i++) {
				// do not remove scalars that correspond to filters
				if (ops[i].depth() == 0 and not odg_ptr->is_expression(i))
					rank0_ops.push_back(i);
			}
			if (rank0_ops.empty())
				return ops;
			odg_ptr = &(odg_ptr->remove_vertices(rank0_ops));
			std::vector<hypertrie::const_Hypertrie<htt_t, allocator_type>> new_ops{};
			for (auto pos : odg_ptr->operands_original_positions()) {
				new_ops.push_back(ops[pos]);
			}
			return new_ops;
		}

		/**
		 * @brief A query provided for evaluation might have some variables already resolved (e.g., SPRARQL EXISTS).
		 * The purpose of resovle_variables is to update the provided query operands and entry according to the
		 * values assigned to resovled variables.
		 * @tparam htt_t
		 * @tparam allocator_type
		 * @param query
		 * @param entry_arg
		 * @return
		 */
		template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
		static void
		resolve_variables(Query<htt_t, allocator_type> &query,
						  OperandDependencyGraph<htt_t, allocator_type> *&odg_ptr,
						  CountedEntry<htt_t> &entry_arg) {
			auto const &resolved_vars = query.resolved_vars();
			// if there are no resolved variables, simply return
			if (resolved_vars.empty())
				return;
			// iterate over the resolved variables
			for (auto const &[var_id, value] : resolved_vars) {
				// if the variable is tracked (e.g., projected or part of an expression), we need to update the entry
				if (query.tracks_variable(var_id))
					entry_arg[query.tracked_var_position(var_id)] = value;
				// we need to update the slice keys of the operands that have this variable id
				// slicing is done later in the query evaluation
				auto var_id_positions_in_operands = odg_ptr->var_ids_positions_in_operands(var_id);
				for (size_t i = 0; i < var_id_positions_in_operands.size(); i++) {
					if (auto op_slice_key = odg_ptr->get_slice_key(i); op_slice_key != nullptr) {
						for (auto pos : var_id_positions_in_operands[i]) {
							(*op_slice_key)[pos] = value;
						}
					}
				}
			}
			// remove the labels from the graph, while persisting vertices without var_ids
			// vertices without var_ids corresponding to operands, will be removed when the slicing takes place
			for (auto const &resolved_var : resolved_vars) {
				odg_ptr = &(odg_ptr->remove_var_id(resolved_var.first, false));
			}
		}
	};

}// namespace dice::query

#endif//QUERY_EVALUATION_HPP
