#include "PrimaryExpressions.hpp"

namespace dice::sparql::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* Variable Expression */
	PrimaryVarExpression::PrimaryVarExpression(Variable variable, size_t var_pos_in_entry)
		: var_pos_in_entry_(var_pos_in_entry), rdf_node_(), variable_(variable) {}

	void PrimaryVarExpression::update_value(detail::Key const &key) {
		rdf_node_ = key[var_pos_in_entry_];
	}

	node_wrapper::NodeWrapper PrimaryVarExpression::evaluate() const {
		return rdf_node_;
	}

	PrimaryVarExpression *PrimaryVarExpression::clone_impl() const {
		return new PrimaryVarExpression(*this);
	}

	std::vector<Variable> PrimaryVarExpression::variables() const {
		return {variable_};
	}

	std::vector<Variable> PrimaryVarExpression::aggregated_variables() const {
		return {};
	}

	std::vector<Variable> PrimaryVarExpression::non_aggregated_variables() const {
		return {variable_};
	}

	/* Literal Expression */
	PrimaryLiteralExpression::PrimaryLiteralExpression(Literal literal)
		: literal_(literal) {}

	void PrimaryLiteralExpression::update_value([[maybe_unused]] detail::Key const &key) {}

	node_wrapper::NodeWrapper PrimaryLiteralExpression::evaluate() const {
		return literal_;
	}

	PrimaryLiteralExpression *PrimaryLiteralExpression::clone_impl() const {
		return new PrimaryLiteralExpression(*this);
	}

	std::vector<Variable> PrimaryLiteralExpression::variables() const {
		return {};
	}

	std::vector<Variable> PrimaryLiteralExpression::aggregated_variables() const {
		return {};
	}

	std::vector<Variable> PrimaryLiteralExpression::non_aggregated_variables() const {
		return {};
	}

	/* IRI Expression */
	PrimaryIRIExpression::PrimaryIRIExpression(IRI iri)
		: iri_(iri) {}

	void PrimaryIRIExpression::update_value([[maybe_unused]] detail::Key const &key) {}

	node_wrapper::NodeWrapper PrimaryIRIExpression::evaluate() const {
		return iri_;
	}

	PrimaryIRIExpression *PrimaryIRIExpression::clone_impl() const {
		return new PrimaryIRIExpression(*this);
	}

	std::vector<Variable> PrimaryIRIExpression::variables() const {
		return {};
	}

	std::vector<Variable> PrimaryIRIExpression::aggregated_variables() const {
		return {};
	}

	std::vector<Variable> PrimaryIRIExpression::non_aggregated_variables() const {
		return {};
	}

	/* BuiltInCall Expression */
	PrimaryBuiltInCallExpression::PrimaryBuiltInCallExpression(std::unique_ptr<SPARQLExpression> expr)
		: built_in_call_(std::move(expr)) {}

	void PrimaryBuiltInCallExpression::update_value(detail::Key const &key) {
		return built_in_call_->update_value(key);
	}

	node_wrapper::NodeWrapper PrimaryBuiltInCallExpression::evaluate() const {
		return built_in_call_->evaluate();
	}

	PrimaryBuiltInCallExpression *PrimaryBuiltInCallExpression::clone_impl() const {
		return new PrimaryBuiltInCallExpression(built_in_call_->clone());
	}

	std::vector<Variable> PrimaryBuiltInCallExpression::variables() const {
		return built_in_call_->variables();
	}

	std::vector<Variable> PrimaryBuiltInCallExpression::aggregated_variables() const {
		return built_in_call_->aggregated_variables();
	}

	std::vector<Variable> PrimaryBuiltInCallExpression::non_aggregated_variables() const {
		return built_in_call_->non_aggregated_variables();
	}

	/* False Expression */
	FalseExpression::FalseExpression() {}

	void FalseExpression::update_value([[maybe_unused]] detail::Key const &key) {}

	node_wrapper::NodeWrapper FalseExpression::evaluate() const {
		return Literal::make_boolean(false);
	}

	FalseExpression *FalseExpression::clone_impl() const {
		return new FalseExpression();
	}

	std::vector<Variable> FalseExpression::variables() const {
		return {};
	}

	std::vector<Variable> FalseExpression::aggregated_variables() const {
		return {};
	}

	std::vector<Variable> FalseExpression::non_aggregated_variables() const {
		return {};
	}

}// namespace dice::sparql2tensor::expressions