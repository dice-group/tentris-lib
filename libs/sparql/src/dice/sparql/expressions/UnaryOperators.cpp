#include "UnaryOperators.hpp"

namespace dice::sparql::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* NotExpression Operator */
	NotExpression::NotExpression(std::unique_ptr<SPARQLExpression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void NotExpression::update_value(detail::Key const &key) {
		primary_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper NotExpression::evaluate() const {
		auto expr_result = node_wrapper::NodeWrapper(primary_expr_->evaluate());
		if (expr_result.null())
			return {};
		auto bool_result = not bool(expr_result); // boolean coercion
		if (bool_result) return Literal::make_boolean(true);
		return Literal::make_boolean(false);
	}

	NotExpression *NotExpression::clone_impl() const {
		return new NotExpression(primary_expr_->clone());
	}

	[[nodiscard]] std::vector<Variable> NotExpression::variables() const {
		return primary_expr_->variables();
	}

	[[nodiscard]] std::vector<Variable> NotExpression::aggregated_variables() const {
		return primary_expr_->aggregated_variables();
	}

	[[nodiscard]] std::vector<Variable> NotExpression::non_aggregated_variables() const {
		return primary_expr_->non_aggregated_variables();
	}


	/* UnaryPlusExpression Operator */
	UnaryPlusExpression::UnaryPlusExpression(std::unique_ptr<SPARQLExpression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void UnaryPlusExpression::update_value(detail::Key const &key) {
		return primary_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper UnaryPlusExpression::evaluate() const {
		return +primary_expr_->evaluate().as_literal();
	}

	UnaryPlusExpression *UnaryPlusExpression::clone_impl() const {
		return new UnaryPlusExpression(primary_expr_->clone());
	}

	[[nodiscard]] std::vector<Variable> UnaryPlusExpression::variables() const {
		return primary_expr_->variables();
	}

	[[nodiscard]] std::vector<Variable> UnaryPlusExpression::aggregated_variables() const {
		return primary_expr_->aggregated_variables();
	}

	[[nodiscard]] std::vector<Variable> UnaryPlusExpression::non_aggregated_variables() const {
		return primary_expr_->non_aggregated_variables();
	}

	/* UnaryMinusExpression Operator */
	UnaryMinusExpression::UnaryMinusExpression(std::unique_ptr<SPARQLExpression> primary_expr)
		: primary_expr_(std::move(primary_expr)) {}

	void UnaryMinusExpression::update_value(detail::Key const &key) {
		primary_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper UnaryMinusExpression::evaluate() const {
		return -primary_expr_->evaluate().as_literal();
	}

	UnaryMinusExpression *UnaryMinusExpression::clone_impl() const {
		return new UnaryMinusExpression(primary_expr_->clone());
	}

	[[nodiscard]] std::vector<Variable> UnaryMinusExpression::variables() const {
		return primary_expr_->variables();
	}

	[[nodiscard]] std::vector<Variable> UnaryMinusExpression::aggregated_variables() const {
		return primary_expr_->aggregated_variables();
	}

	[[nodiscard]] std::vector<Variable> UnaryMinusExpression::non_aggregated_variables() const {
		return primary_expr_->non_aggregated_variables();
	}


}// namespace dice::sparql2tensor::expressions