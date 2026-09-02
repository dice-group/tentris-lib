#ifndef QUERY_CARDINALITYESTIMATION_HPP
#define QUERY_CARDINALITYESTIMATION_HPP

#include <cmath>

#include <boost/container/flat_set.hpp>

#include <dice/query/operators/Operator_predeclare.hpp>

namespace dice::query::operators {


	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type, bool Optional = false>
	struct CardinalityEstimation {

		static char getMinCardLabel(OperandDependencyGraph<htt_t, allocator_type> &odg,
									const std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> &operands,
									Query<htt_t, allocator_type> const &query) {
			// determine the set of variables ids to be considered for cardinality estimation
			boost::container::flat_set<char> const *var_ids_candidate_set = nullptr;
			if constexpr (not Optional) {
				// in case there is not any optional parts: consider all var_ids
				var_ids_candidate_set = &odg.operands_var_ids_set();
			}
			else {
				// in case there is an optional part: consider only var_ids in ISC
				var_ids_candidate_set = &odg.non_optional_var_ids_set();
			}
			// if there is only a single var_id, return it
			if (var_ids_candidate_set->size() == 1)
				return *var_ids_candidate_set->begin();

			char min_var = *var_ids_candidate_set->begin();
			double min_cardinality = std::numeric_limits<double>::infinity();

			for (auto const &var : *var_ids_candidate_set) {
				// skip lonely vars that are not projected (they will be counted out)
				if (odg.lonely_var_ids().contains(var) and
					std::find(query.tracked_vars().begin(), query.tracked_vars().end(), var) == query.tracked_vars().end())
					continue;
				// calculate cardinality
				const double label_cardinality = calcCard(odg, operands, var);
				// update min cardinality
				if (label_cardinality < min_cardinality) {
					min_cardinality = label_cardinality;
					min_var = var;
				}
				// break ties using filters and assignments
				else if (label_cardinality == min_cardinality) {
					if (odg.var_id_in_expression(var) and not odg.var_id_in_expression(min_var))
						min_var = var;
				}
			}
			return min_var;
		}

		static double estimate(OperandDependencyGraph<htt_t, allocator_type> &odg,
							   const std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> &operands,
							   Query<htt_t, allocator_type> const &q) {
			auto const &operandsLabelSet = odg.operands_var_ids_set();
			std::vector<double> operand_sizes(operands.size());
			for (size_t i = 0; i < operands.size(); ++i)
				operand_sizes[i] = operands[i].size();

			std::vector<double> var_factors;
			for (auto const &var : operandsLabelSet)
				if (not odg.lonely_var_ids().contains(var) or
					std::find(q.tracked_vars().begin(), q.tracked_vars().end(), var) != q.tracked_vars().end())
					var_factors.push_back(calcCard(odg, operands, var));

			double card = 1;
			bool order = operand_sizes.size() < var_factors.size();
			auto &shorter_vec = order ? operand_sizes : var_factors;
			auto &longer_vec = order ? var_factors : operand_sizes;

			size_t i = 0;
			for (; i < shorter_vec.size(); ++i)
				card *= shorter_vec[i] * longer_vec[i];

			for (; i < longer_vec.size(); ++i)
				card *= longer_vec[i];

			return card;
		}

	protected:
		static double calcCard(OperandDependencyGraph<htt_t, allocator_type> &odg,
							   const std::vector<::dice::hypertrie::const_Hypertrie<htt_t, allocator_type>> &operands,
							   const char var) {
			// get operands that have the label
			std::vector<uint8_t> const *operands_positions = nullptr;
			if constexpr (not Optional) {
				// in case there is not any optional parts: consider all operands
				operands_positions = &odg.operands_with_var_id(var);
			} else {
				// in case there is an optional part: consider operands in ISC
				operands_positions = &odg.isc_operands_with_var_id(var);
			}

			std::vector<double> op_dim_cardinalities(operands_positions->size(), 1.0);
			auto min_dim_card = std::numeric_limits<size_t>::max();
			boost::container::flat_set<size_t> operands_identifiers{};

			auto const &var_positions = odg.var_ids_positions_in_operands(var);
			// iterate the operands that hold the label
			for (size_t i = 0; i < operands_positions->size(); ++i) {
				auto const &op_pos = (*operands_positions)[i];
				auto const &operand = operands[op_pos];
				auto const operand_id = operand.hash();
				auto const op_dim_cards = operand.get_cards(var_positions[op_pos]);
				auto const min_op_dim_card = *std::min_element(op_dim_cards.cbegin(), op_dim_cards.cend());
				auto const max_op_dim_card = *std::max_element(op_dim_cards.cbegin(), op_dim_cards.cend());

				operands_identifiers.insert(operand_id);

				// update minimal dimension cardinality
				if (min_op_dim_card < min_dim_card)
					min_dim_card = min_op_dim_card;

				op_dim_cardinalities[i] = double(max_op_dim_card);
			}

			auto const min_dim_card_d = double(min_dim_card);

			double card = std::accumulate(op_dim_cardinalities.cbegin(), op_dim_cardinalities.cend(), double(1),
										  [&](double a, double b) {
											  return a * min_dim_card_d / b;
										  }) / operands_identifiers.size();
			return card;
		}
	};
}// namespace dice::query::operators
#endif//QUERY_CARDINALITYESTIMATION_HPP
