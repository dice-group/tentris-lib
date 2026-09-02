#include "ArithmeticExpressions.hpp"

namespace dice::sparql::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* Addition */
	AdditionExpression::AdditionExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_expr_(std::move(lhs)), rhs_expr_(std::move(rhs)) {}

	void AdditionExpression::update_value(detail::Key const &key) {
		lhs_expr_->update_value(key);
		rhs_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper AdditionExpression::evaluate() const {
		return lhs_expr_->evaluate().as_literal() + rhs_expr_->evaluate().as_literal();
	}

	std::vector<rdf4cpp::rdf::query::Variable> AdditionExpression::variables() const {
		auto l_vars = lhs_expr_->variables();
		auto r_vars = rhs_expr_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::vector<rdf4cpp::rdf::query::Variable> AdditionExpression::aggregated_variables() const {
		auto l_vars = lhs_expr_->aggregated_variables();
		auto r_vars = rhs_expr_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::vector<rdf4cpp::rdf::query::Variable> AdditionExpression::non_aggregated_variables() const {
		auto l_vars = lhs_expr_->non_aggregated_variables();
		auto r_vars = rhs_expr_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	AdditionExpression *AdditionExpression::clone_impl() const {
		return new AdditionExpression(lhs_expr_->clone(), rhs_expr_->clone());
	}

	/* Subtraction */
	SubtractionExpression::SubtractionExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_expr_(std::move(lhs)), rhs_expr_(std::move(rhs)) {}

	void SubtractionExpression::update_value(detail::Key const &key) {
		lhs_expr_->update_value(key);
		rhs_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper SubtractionExpression::evaluate() const {
		return lhs_expr_->evaluate().as_literal() - rhs_expr_->evaluate().as_literal();
	}

	std::vector<rdf4cpp::rdf::query::Variable> SubtractionExpression::variables() const {
		auto l_vars = lhs_expr_->variables();
		auto r_vars = rhs_expr_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::vector<rdf4cpp::rdf::query::Variable> SubtractionExpression::aggregated_variables() const {
		auto l_vars = lhs_expr_->aggregated_variables();
		auto r_vars = rhs_expr_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::vector<rdf4cpp::rdf::query::Variable> SubtractionExpression::non_aggregated_variables() const {
		auto l_vars = lhs_expr_->non_aggregated_variables();
		auto r_vars = rhs_expr_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	SubtractionExpression *SubtractionExpression::clone_impl() const {
		return new SubtractionExpression(lhs_expr_->clone(), rhs_expr_->clone());
	}

	/* Multiplication */
	MultiplicationExpression::MultiplicationExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_expr_(std::move(lhs)), rhs_expr_(std::move(rhs)) {}

	void MultiplicationExpression::update_value(detail::Key const &key) {
		lhs_expr_->update_value(key);
		rhs_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper MultiplicationExpression::evaluate() const {
		return lhs_expr_->evaluate().as_literal() * rhs_expr_->evaluate().as_literal();
	}

	std::vector<rdf4cpp::rdf::query::Variable> MultiplicationExpression::variables() const {
		auto l_vars = lhs_expr_->variables();
		auto r_vars = rhs_expr_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::vector<rdf4cpp::rdf::query::Variable> MultiplicationExpression::aggregated_variables() const {
		auto l_vars = lhs_expr_->aggregated_variables();
		auto r_vars = rhs_expr_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::vector<rdf4cpp::rdf::query::Variable> MultiplicationExpression::non_aggregated_variables() const {
		auto l_vars = lhs_expr_->non_aggregated_variables();
		auto r_vars = rhs_expr_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	MultiplicationExpression *MultiplicationExpression::clone_impl() const {
		return new MultiplicationExpression(lhs_expr_->clone(), rhs_expr_->clone());
	}

	/* Division */
	DivisionExpression::DivisionExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_expr_(std::move(lhs)), rhs_expr_(std::move(rhs)) {}

	void DivisionExpression::update_value(detail::Key const &key) {
		lhs_expr_->update_value(key);
		rhs_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper DivisionExpression::evaluate() const {
		return lhs_expr_->evaluate().as_literal() / rhs_expr_->evaluate().as_literal();
	}

	std::vector<rdf4cpp::rdf::query::Variable> DivisionExpression::variables() const {
		auto l_vars = lhs_expr_->variables();
		auto r_vars = rhs_expr_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::vector<rdf4cpp::rdf::query::Variable> DivisionExpression::aggregated_variables() const {
		auto l_vars = lhs_expr_->aggregated_variables();
		auto r_vars = rhs_expr_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::vector<rdf4cpp::rdf::query::Variable> DivisionExpression::non_aggregated_variables() const {
		auto l_vars = lhs_expr_->non_aggregated_variables();
		auto r_vars = rhs_expr_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	DivisionExpression *DivisionExpression::clone_impl() const {
		return new DivisionExpression(lhs_expr_->clone(), rhs_expr_->clone());
	}

}// namespace dice::sparql::expressions