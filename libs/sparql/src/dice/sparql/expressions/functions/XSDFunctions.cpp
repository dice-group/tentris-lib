#include "XSDFunctions.hpp"

namespace dice::sparql::expressions::functions {

	using namespace rdf4cpp::rdf;

	Constructor::Constructor(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	void Constructor::update_value(detail::Key const &key) {
		op_expr_->update_value(key);
	}

	std::vector<rdf4cpp::rdf::query::Variable> Constructor::variables() const {
		return op_expr_->variables();
	}

	std::vector<rdf4cpp::rdf::query::Variable> Constructor::aggregated_variables() const {
		return op_expr_->aggregated_variables();
	}

	std::vector<rdf4cpp::rdf::query::Variable> Constructor::non_aggregated_variables() const {
		return op_expr_->non_aggregated_variables();
	}

	StringConstructor::StringConstructor(std::unique_ptr<SPARQLExpression> op_expr)
		: Constructor(std::move(op_expr)) {}

	node_wrapper::NodeWrapper StringConstructor::evaluate() const {
		auto expr_result = op_expr_->evaluate();
		if (auto expr_result_iri = expr_result.as_iri(); not expr_result_iri.null())
			return Literal::make_simple(expr_result_iri.identifier());
		return expr_result.as_literal().cast<rdf4cpp::rdf::datatypes::xsd::String>();
	}

	StringConstructor *StringConstructor::clone_impl() const {
		return new StringConstructor(op_expr_->clone());
	}

	IntegerConstructor::IntegerConstructor(std::unique_ptr<SPARQLExpression> op_expr)
		: Constructor(std::move(op_expr)) {}

	node_wrapper::NodeWrapper IntegerConstructor::evaluate() const {
		return op_expr_->evaluate().as_literal().cast<rdf4cpp::rdf::datatypes::xsd::Integer>();
	}

	IntegerConstructor *IntegerConstructor::clone_impl() const {
		return new IntegerConstructor(op_expr_->clone());
	}

	DecimalConstructor::DecimalConstructor(std::unique_ptr<SPARQLExpression> op_expr)
		: Constructor(std::move(op_expr)) {}

	node_wrapper::NodeWrapper DecimalConstructor::evaluate() const {
		return op_expr_->evaluate().as_literal().cast<rdf4cpp::rdf::datatypes::xsd::Decimal>();
	}

	DecimalConstructor *DecimalConstructor::clone_impl() const {
		return new DecimalConstructor(op_expr_->clone());
	}

	DoubleConstructor::DoubleConstructor(std::unique_ptr<SPARQLExpression> op_expr)
		: Constructor(std::move(op_expr)) {}

	node_wrapper::NodeWrapper DoubleConstructor::evaluate() const {
		return op_expr_->evaluate().as_literal().cast<rdf4cpp::rdf::datatypes::xsd::Double>();
	}

	DoubleConstructor *DoubleConstructor::clone_impl() const {
		return new DoubleConstructor(op_expr_->clone());
	}

	FloatConstructor::FloatConstructor(std::unique_ptr<SPARQLExpression> op_expr)
		: Constructor(std::move(op_expr)) {}

	node_wrapper::NodeWrapper FloatConstructor::evaluate() const {
		return op_expr_->evaluate().as_literal().cast<rdf4cpp::rdf::datatypes::xsd::Float>();
	}

	FloatConstructor *FloatConstructor::clone_impl() const {
		return new FloatConstructor(op_expr_->clone());
	}

	BooleanConstructor::BooleanConstructor(std::unique_ptr<SPARQLExpression> op_expr)
		: Constructor(std::move(op_expr)) {}

	node_wrapper::NodeWrapper BooleanConstructor::evaluate() const {
		return op_expr_->evaluate().as_literal().cast<rdf4cpp::rdf::datatypes::xsd::Boolean>();
	}

	BooleanConstructor *BooleanConstructor::clone_impl() const {
		return new BooleanConstructor(op_expr_->clone());
	}

	DateTimeConstructor::DateTimeConstructor(std::unique_ptr<SPARQLExpression> op_expr)
		: Constructor(std::move(op_expr)) {}

	node_wrapper::NodeWrapper DateTimeConstructor::evaluate() const {
		// currently not supported
		assert(false);
		// return op_expr_->evaluate().as_literal().cast<rdf4cpp::rdf::datatypes::xsd::DateTime>()
	}

	DateTimeConstructor *DateTimeConstructor::clone_impl() const {
		return new DateTimeConstructor(op_expr_->clone());
	}

}// namespace dice::sparql::expressions::functions