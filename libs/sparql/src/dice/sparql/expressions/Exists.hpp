#ifndef TENTRIS_QUERY_SPARQL_EXISTS_HPP
#define TENTRIS_QUERY_SPARQL_EXISTS_HPP

#include <dice/sparql/expressions/Expression.hpp>

namespace dice::sparql::expressions {

	class Exists : public SPARQLExpression {
	private:
		mutable detail::Query sub_query_;
		bool not_exists_;
		std::vector<rdf4cpp::rdf::query::Variable> variables_;
		boost::container::flat_map<char, size_t> var_ids_positions_;

	public:
		Exists(std::vector<rdf4cpp::rdf::query::Variable> variables,
			   boost::container::flat_map<char, size_t> var_ids_positions,
			   detail::Query sub_query,
			   bool not_exists);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate(detail::EvaluationContext &eval_ctx) const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;

	protected:
		[[nodiscard]] Exists *clone_impl() const override;
	};

}// namespace dice::sparql2tensor::expressions

#endif//TENTRIS_QUERY_SPARQL_EXISTS_HPP
