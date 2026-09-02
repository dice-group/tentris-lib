#ifndef DICE_SPARQL_XSDFUNCTIONS_HPP
#define DICE_SPARQL_XSDFUNCTIONS_HPP

#include <dice/sparql/expressions/Expression.hpp>

namespace dice::sparql::expressions::functions {

	class Constructor : public SPARQLExpression {
	protected:
		std::unique_ptr<SPARQLExpression> op_expr_;

	public:
		explicit Constructor(std::unique_ptr<SPARQLExpression> op_expr);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	};

	class StringConstructor : public Constructor {

	public:
		explicit StringConstructor(std::unique_ptr<SPARQLExpression> op_expr);
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;

	protected:
		[[nodiscard]] StringConstructor *clone_impl() const override;
	};

	class IntegerConstructor : public Constructor {

	public:
		explicit IntegerConstructor(std::unique_ptr<SPARQLExpression> op_expr);
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;

	protected:
		[[nodiscard]] IntegerConstructor *clone_impl() const override;
	};

	class DecimalConstructor : public Constructor {

	public:
		explicit DecimalConstructor(std::unique_ptr<SPARQLExpression> op_expr);
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;

	protected:
		[[nodiscard]] DecimalConstructor *clone_impl() const override;
	};

	class DoubleConstructor : public Constructor {

	public:
		explicit DoubleConstructor(std::unique_ptr<SPARQLExpression> op_expr);
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;

	protected:
		[[nodiscard]] DoubleConstructor *clone_impl() const override;
	};

	class FloatConstructor : public Constructor {

	public:
		explicit FloatConstructor(std::unique_ptr<SPARQLExpression> op_expr);
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;

	protected:
		[[nodiscard]] FloatConstructor *clone_impl() const override;
	};

	class BooleanConstructor : public Constructor {

	public:
		explicit BooleanConstructor(std::unique_ptr<SPARQLExpression> op_expr);
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;

	protected:
		[[nodiscard]] BooleanConstructor *clone_impl() const override;
	};

	class DateTimeConstructor : public Constructor {

	public:
		explicit DateTimeConstructor(std::unique_ptr<SPARQLExpression> op_expr);
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;

	protected:
		[[nodiscard]] DateTimeConstructor *clone_impl() const override;
	};

}// namespace dice::sparql::expressions::functions

#endif//DICE_SPARQL_XSDFUNCTIONS_HPP
