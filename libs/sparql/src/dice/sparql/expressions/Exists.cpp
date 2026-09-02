#include "Exists.hpp"

#include <robin_hood.h>

namespace dice::sparql::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;


	Exists::Exists(std::vector<rdf4cpp::rdf::query::Variable> variables,
				   boost::container::flat_map<char, size_t> var_ids_positions,
				   detail::Query sub_query,
				   bool not_exists)
		: sub_query_(std::move(sub_query)), not_exists_(not_exists),
		  variables_(std::move(variables)), var_ids_positions_(std::move(var_ids_positions)) {}

	void Exists::update_value(const detail::Key &key) {
		for (auto const &[var_id, pos] : var_ids_positions_) {
			sub_query_.assign_value_to_var(var_id, key[pos]);
		}
	}

	node_wrapper::NodeWrapper Exists::evaluate() const { assert(false); return {};}

	node_wrapper::NodeWrapper Exists::evaluate(detail::EvaluationContext &eval_ctx) const {
		auto generator_iter = detail::QueryEvaluation::evaluate(sub_query_, eval_ctx);
		auto first = generator_iter.begin();
		bool has_solutions = false;
		if (first != generator_iter.end())
			if ((*first).value() > 0)
				has_solutions = true;
		return Literal::make_typed_from_value<datatypes::xsd::Boolean>(not_exists_ ? !has_solutions : has_solutions);
	}

	std::vector<rdf4cpp::rdf::query::Variable> Exists::variables() const {
		return variables_;
	}

	std::vector<rdf4cpp::rdf::query::Variable> Exists::aggregated_variables() const {
		assert(false);
		return {};
	}

	std::vector<rdf4cpp::rdf::query::Variable> Exists::non_aggregated_variables() const {
		assert(false);
		return {};
	}

	Exists *Exists::clone_impl() const {
		return new Exists(variables_, var_ids_positions_, sub_query_, not_exists_);
	}


}// namespace dice::sparql2tensor::expressions