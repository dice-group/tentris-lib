#ifndef TENTRIS_QUERY_SPARQL_PRIMARYEXPRESSIONS_HPP
#define TENTRIS_QUERY_SPARQL_PRIMARYEXPRESSIONS_HPP

#include <dice/sparql/expressions/Expression.hpp>

namespace dice::sparql::expressions {

	/* PrimaryExpression for Variables (https://www.w3.org/TR/sparql11-query/#rPrimaryExpression) */
	class PrimaryVarExpression : public SPARQLExpression {
	private:
		size_t var_pos_in_entry_;
		rdf4cpp::rdf::Node rdf_node_;
		rdf4cpp::rdf::query::Variable variable_;
	public:
		explicit PrimaryVarExpression(rdf4cpp::rdf::query::Variable variable, size_t var_pos_in_entry);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] PrimaryVarExpression *clone_impl() const override;
	};

	/* PrimaryExpression for RDFLiterals, NumericLiterals and BooleanLiterals (https://www.w3.org/TR/sparql11-query/#rPrimaryExpression) */
	struct PrimaryLiteralExpression : public SPARQLExpression {
	private:
		rdf4cpp::rdf::Literal literal_;
	public:
		explicit PrimaryLiteralExpression(rdf4cpp::rdf::Literal literal);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] PrimaryLiteralExpression *clone_impl() const override;
	};

	/* PrimaryExpression for IRIs (https://www.w3.org/TR/sparql11-query/#rPrimaryExpression) */
	struct PrimaryIRIExpression : public SPARQLExpression {
	private:
		rdf4cpp::rdf::IRI iri_;
	public:
		explicit PrimaryIRIExpression(rdf4cpp::rdf::IRI iri);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] PrimaryIRIExpression *clone_impl() const override;
	};

	/* PrimaryExpression for BuiltInCalls (https://www.w3.org/TR/sparql11-query/#rPrimaryExpression) */
	struct PrimaryBuiltInCallExpression : public SPARQLExpression {
	private:
		std::unique_ptr<SPARQLExpression> built_in_call_;
	public:
		explicit PrimaryBuiltInCallExpression(std::unique_ptr<SPARQLExpression> built_in_call);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] PrimaryBuiltInCallExpression *clone_impl() const override;
	};

	/* Always False */
	struct FalseExpression : public SPARQLExpression {
	public:
		explicit FalseExpression();
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	protected:
		[[nodiscard]] FalseExpression *clone_impl() const override;
	};

}//namespace dice::sparql2tensor::expressions


#endif//TENTRIS_QUERY_SPARQL_PRIMARYEXPRESSIONS_HPP
