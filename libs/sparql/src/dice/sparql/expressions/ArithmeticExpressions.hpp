#ifndef QUERY_SPARQL_ARITHMETICEXPRESSIONS_HPP
#define QUERY_SPARQL_ARITHMETICEXPRESSIONS_HPP

#include <dice/sparql/expressions/Expression.hpp>

namespace dice::sparql::expressions {

	class AdditionExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_expr_;
		std::unique_ptr<SPARQLExpression> rhs_expr_;
	public:
		explicit AdditionExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] AdditionExpression *clone_impl() const override;
	};

	class SubtractionExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_expr_;
		std::unique_ptr<SPARQLExpression> rhs_expr_;
	public:
		explicit SubtractionExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] SubtractionExpression *clone_impl() const override;
	};

	class MultiplicationExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_expr_;
		std::unique_ptr<SPARQLExpression> rhs_expr_;
	public:
		explicit MultiplicationExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] MultiplicationExpression *clone_impl() const override;
	};

	class DivisionExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> lhs_expr_;
		std::unique_ptr<SPARQLExpression> rhs_expr_;
	public:
		explicit DivisionExpression(std::unique_ptr<SPARQLExpression> lhs, std::unique_ptr<SPARQLExpression> rhs);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] DivisionExpression *clone_impl() const override;
	};

}

#endif//QUERY_SPARQL_ARITHMETICEXPRESSIONS_HPP
