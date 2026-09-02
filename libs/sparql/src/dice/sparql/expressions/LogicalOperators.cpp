#include "LogicalOperators.hpp"

namespace dice::sparql::expressions {

	using namespace rdf4cpp::rdf;

	/* LogicalAnd Operator */
	LogicalAndExpression::LogicalAndExpression(std::vector<std::unique_ptr<SPARQLExpression>> op_expressions)
		: op_expressions_(std::move(op_expressions)) {}

	void LogicalAndExpression::update_value(detail::Key const &key) {
		for (auto const &expr : op_expressions_) {
			expr->update_value(key);
		}
	}

	node_wrapper::NodeWrapper LogicalAndExpression::evaluate() const {
		bool contains_error = false;
		for (auto const &expr : op_expressions_) {
			auto expr_result = expr->evaluate();
			if (expr_result.null()) {
				contains_error = true;
				continue;
			}
			if (not bool(expr_result)) {
				return Literal::make_boolean(false);
			}
		}
		if (contains_error)
			return {};
		return Literal::make_boolean(true);
	}

	LogicalAndExpression *LogicalAndExpression::clone_impl() const {
		std::vector<std::unique_ptr<SPARQLExpression>> clones{};
		for (auto const &expr : op_expressions_) {
			clones.push_back(expr->clone());
		}
		return new LogicalAndExpression(std::move(clones));
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalAndExpression::variables() const {
		auto variables = op_expressions_[0]->variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalAndExpression::aggregated_variables() const {
		auto variables = op_expressions_[0]->aggregated_variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->aggregated_variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalAndExpression::non_aggregated_variables() const {
		auto variables = op_expressions_[0]->non_aggregated_variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->non_aggregated_variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	[[nodiscard]] std::vector<std::unique_ptr<SPARQLExpression>> &LogicalAndExpression::expressions() {
		return op_expressions_;
	}

	/* LogicalOr Operator */
	LogicalOrExpression::LogicalOrExpression(std::vector<std::unique_ptr<SPARQLExpression>> op_expressions)
		: op_expressions_(std::move(op_expressions)) {}

	void LogicalOrExpression::update_value(detail::Key const &key) {
		for (auto const &expr : op_expressions_) {
			expr->update_value(key);
		}
	}

	node_wrapper::NodeWrapper LogicalOrExpression::evaluate() const {
		bool contains_error = false;
		for (auto const &expr : op_expressions_) {
			auto expr_result = expr->evaluate();
			if (expr_result.null()) {
				contains_error = true;
				continue;
			}
			if (bool(expr_result)) {
				return Literal::make_boolean(true);
			}
		}
		if (contains_error)
			return {};
		return Literal::make_boolean(false);
	}

	LogicalOrExpression *LogicalOrExpression::clone_impl() const {
		std::vector<std::unique_ptr<SPARQLExpression>> clones{};
		for (auto const &expr : op_expressions_) {
			clones.push_back(expr->clone());
		}
		return new LogicalOrExpression(std::move(clones));
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalOrExpression::variables() const {
		auto variables = op_expressions_[0]->variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalOrExpression::aggregated_variables() const {
		auto variables = op_expressions_[0]->aggregated_variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->aggregated_variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LogicalOrExpression::non_aggregated_variables() const {
		auto variables = op_expressions_[0]->non_aggregated_variables();
		for (size_t i = 1; i < op_expressions_.size(); i++) {
			auto vars = op_expressions_[i]->non_aggregated_variables();
			variables.insert(variables.end(), vars.begin(), vars.end());
		}
		return variables;
	}

	std::partial_ordering compare_impl(rdf4cpp::rdf::Node lhs, rdf4cpp::rdf::Node rhs) {
		auto lhs_lit = lhs.as_literal();
		auto rhs_lit = rhs.as_literal();
		// use the dedicated operator for literals
		if (not lhs_lit.null() and not rhs_lit.null()) {
			return (lhs_lit <=> rhs_lit);
		}
		return lhs <=> rhs;
	}

	/* Equals Operator */
	EqualsExpression::EqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void EqualsExpression::update_value(detail::Key const &key) {
		lhs_op_->update_value(key);
		rhs_op_->update_value(key);
	}

	node_wrapper::NodeWrapper EqualsExpression::evaluate() const {
		return Literal::make_boolean(compare_impl(lhs_op_->evaluate(), rhs_op_->evaluate()) == std::partial_ordering::equivalent);
	}

	EqualsExpression *EqualsExpression::clone_impl() const {
		return new EqualsExpression(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> EqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> EqualsExpression::aggregated_variables() const {
		auto l_vars = lhs_op_->aggregated_variables();
		auto r_vars = rhs_op_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> EqualsExpression::non_aggregated_variables() const {
		auto l_vars = lhs_op_->non_aggregated_variables();
		auto r_vars = rhs_op_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	std::unique_ptr<SPARQLExpression> EqualsExpression::lhs_expression() const {
		return lhs_op_->clone();
	}

	std::unique_ptr<SPARQLExpression> EqualsExpression::rhs_expression() const {
		return rhs_op_->clone();
	}

	/* NotEquals Operator */
	NotEqualsExpression::NotEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void NotEqualsExpression::update_value(detail::Key const &key) {
		lhs_op_->update_value(key);
		rhs_op_->update_value(key);
	}

	node_wrapper::NodeWrapper NotEqualsExpression::evaluate() const {
		return Literal::make_boolean(compare_impl(lhs_op_->evaluate(), rhs_op_->evaluate()) != std::partial_ordering::equivalent);
	}

	NotEqualsExpression *NotEqualsExpression::clone_impl() const {
		return new NotEqualsExpression(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> NotEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> NotEqualsExpression::aggregated_variables() const {
		auto l_vars = lhs_op_->aggregated_variables();
		auto r_vars = rhs_op_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> NotEqualsExpression::non_aggregated_variables() const {
		auto l_vars = lhs_op_->non_aggregated_variables();
		auto r_vars = rhs_op_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* Less Operator */
	LessExpression::LessExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void LessExpression::update_value(detail::Key const &key) {
		lhs_op_->update_value(key);
		rhs_op_->update_value(key);
	}

	node_wrapper::NodeWrapper LessExpression::evaluate() const {
		return Literal::make_boolean(compare_impl(lhs_op_->evaluate(), rhs_op_->evaluate()) == std::partial_ordering::less);
	}

	LessExpression *LessExpression::clone_impl() const {
		return new LessExpression(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessExpression::aggregated_variables() const {
		auto l_vars = lhs_op_->aggregated_variables();
		auto r_vars = rhs_op_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessExpression::non_aggregated_variables() const {
		auto l_vars = lhs_op_->non_aggregated_variables();
		auto r_vars = rhs_op_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* Greater Operator */
	GreaterExpression::GreaterExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void GreaterExpression::update_value(detail::Key const &key) {
		lhs_op_->update_value(key);
		rhs_op_->update_value(key);
	}

	node_wrapper::NodeWrapper GreaterExpression::evaluate() const {
		return Literal::make_boolean(compare_impl(lhs_op_->evaluate(), rhs_op_->evaluate()) == std::partial_ordering::greater);
	}

	GreaterExpression *GreaterExpression::clone_impl() const {
		return new GreaterExpression(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterExpression::aggregated_variables() const {
		auto l_vars = lhs_op_->aggregated_variables();
		auto r_vars = rhs_op_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterExpression::non_aggregated_variables() const {
		auto l_vars = lhs_op_->non_aggregated_variables();
		auto r_vars = rhs_op_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* LessEquals Operator */
	LessEqualsExpression::LessEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void LessEqualsExpression::update_value(detail::Key const &key) {
		lhs_op_->update_value(key);
		rhs_op_->update_value(key);
	}

	node_wrapper::NodeWrapper LessEqualsExpression::evaluate() const {
		auto p_ordering = compare_impl(lhs_op_->evaluate(), rhs_op_->evaluate());
		return Literal::make_boolean(p_ordering == std::partial_ordering::equivalent or p_ordering == std::partial_ordering::less);
	}

	LessEqualsExpression *LessEqualsExpression::clone_impl() const {
		return new LessEqualsExpression(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessEqualsExpression::aggregated_variables() const {
		auto l_vars = lhs_op_->aggregated_variables();
		auto r_vars = rhs_op_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> LessEqualsExpression::non_aggregated_variables() const {
		auto l_vars = lhs_op_->non_aggregated_variables();
		auto r_vars = rhs_op_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* GreaterEquals Operator */
	GreaterEqualsExpression::GreaterEqualsExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)) {}

	void GreaterEqualsExpression::update_value(detail::Key const &key) {
		lhs_op_->update_value(key);
		rhs_op_->update_value(key);
	}

	node_wrapper::NodeWrapper GreaterEqualsExpression::evaluate() const {
		auto p_ordering = compare_impl(lhs_op_->evaluate(), rhs_op_->evaluate());
		return Literal::make_boolean(p_ordering == std::partial_ordering::equivalent or p_ordering == std::partial_ordering::greater);
	}

	GreaterEqualsExpression *GreaterEqualsExpression::clone_impl() const {
		return new GreaterEqualsExpression(lhs_op_->clone(), rhs_op_->clone());
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterEqualsExpression::variables() const {
		auto l_vars = lhs_op_->variables();
		auto r_vars = rhs_op_->variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterEqualsExpression::aggregated_variables() const {
		auto l_vars = lhs_op_->aggregated_variables();
		auto r_vars = rhs_op_->aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> GreaterEqualsExpression::non_aggregated_variables() const {
		auto l_vars = lhs_op_->non_aggregated_variables();
		auto r_vars = rhs_op_->non_aggregated_variables();
		l_vars.insert(l_vars.end(), r_vars.begin(), r_vars.end());
		return l_vars;
	}

	/* InExpressionList Operator */
	InExpressionList::InExpressionList(std::unique_ptr<SPARQLExpression> lhs, std::vector<std::unique_ptr<SPARQLExpression>> rhs, bool not_in)
		: lhs_op_(std::move(lhs)), rhs_op_(std::move(rhs)), not_in_(not_in) {}

	void InExpressionList::update_value(detail::Key const &key) {
		lhs_op_->update_value(key);
		for (auto &rhs_expr : rhs_op_) {
			rhs_expr->update_value(key);
		}
	}

	node_wrapper::NodeWrapper InExpressionList::evaluate() const {
		bool is_in = false;
		auto lhs_result = lhs_op_->evaluate();
		for (auto &rhs_expr : rhs_op_) {
			if (lhs_result == rhs_expr->evaluate()) {
				is_in = true;
				break;
			}
		}
		if ((not not_in_ and is_in) or (not_in_ and not is_in))
			return Literal::make_boolean(true);
		return Literal::make_boolean(false);
	}

	InExpressionList *InExpressionList::clone_impl() const {
		std::vector<std::unique_ptr<SPARQLExpression>> rhs_expressions{};
		for (auto &rhs_expr : rhs_op_) {
			rhs_expressions.push_back(rhs_expr->clone());
		}
		return new InExpressionList(lhs_op_->clone(), std::move(rhs_expressions));
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> InExpressionList::variables() const {
		auto l_vars = lhs_op_->variables();
		for (auto &rhs_expr : rhs_op_) {
			auto const &expr_vars = rhs_expr->variables();
			l_vars.insert(l_vars.end(), expr_vars.begin(), expr_vars.end());
		}
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> InExpressionList::aggregated_variables() const {
		auto l_vars = lhs_op_->aggregated_variables();
		for (auto &rhs_expr : rhs_op_) {
			auto const &expr_vars = rhs_expr->aggregated_variables();
			l_vars.insert(l_vars.end(), expr_vars.begin(), expr_vars.end());
		}
		return l_vars;
	}

	[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> InExpressionList::non_aggregated_variables() const {
		auto l_vars = lhs_op_->non_aggregated_variables();
		for (auto &rhs_expr : rhs_op_) {
			auto const &expr_vars = rhs_expr->non_aggregated_variables();
			l_vars.insert(l_vars.end(), expr_vars.begin(), expr_vars.end());
		}
		return l_vars;
	}

}// namespace dice::sparql2tensor::expressions