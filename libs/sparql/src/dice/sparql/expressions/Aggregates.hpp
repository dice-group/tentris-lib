#ifndef TENTRIS_QUERY_SPARQL_AGGREGATES_HPP
#define TENTRIS_QUERY_SPARQL_AGGREGATES_HPP

#include <dice/sparql/expressions/Expression.hpp>

namespace dice::sparql::expressions {

	/* https://www.w3.org/TR/sparql11-query/#rAggregate */
	class Aggregate : public SPARQLExpression {
	protected:
		std::unique_ptr<SPARQLExpression> op_expr_;
	public:
		explicit Aggregate(std::unique_ptr<SPARQLExpression> op_expr);
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	};

	class CountStar : public Aggregate {
	private:
		size_t count_ = 0;
	public:
		explicit CountStar(size_t count = 0);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] CountStar *clone_impl() const override;
	};

	class CountStarDistinct : public Aggregate {
	private:
		// stores hashes of keys (detail::Key)
		boost::container::flat_set<size_t> keys_;
	public:
		explicit CountStarDistinct(boost::container::flat_set<size_t> keys_ = {});
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
	protected:
		[[nodiscard]] CountStarDistinct *clone_impl() const override;
	};

	class Count : public Aggregate {
	private:
		size_t count_ = 0;
	public:
		explicit Count(std::unique_ptr<SPARQLExpression> expr, size_t count = 0);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Count *clone_impl() const override;
	};

	class CountDistinct : public Aggregate {
	private:
		boost::container::flat_set<rdf4cpp::rdf::Node> rdf_nodes_;
	public:
		explicit CountDistinct(std::unique_ptr<SPARQLExpression> expr, boost::container::flat_set<rdf4cpp::rdf::Node> rdf_nodes = {});
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] CountDistinct *clone_impl() const override;
	};

	class Sum : public Aggregate {
	private:
		rdf4cpp::rdf::Literal rdf_literal_;
	public:
		explicit Sum(std::unique_ptr<SPARQLExpression> expr,
					 rdf4cpp::rdf::Literal rdf_node = rdf4cpp::rdf::Literal::make_typed_from_value<rdf4cpp::rdf::datatypes::xsd::Integer>(0));
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Sum *clone_impl() const override;
	};

	class SumDistinct : public Aggregate {
	private:
		rdf4cpp::rdf::Literal rdf_literal_;
		boost::container::flat_set<rdf4cpp::rdf::Literal> rdf_literals_;
	public:
		explicit SumDistinct(std::unique_ptr<SPARQLExpression> expr,
							 rdf4cpp::rdf::Literal rdf_node = rdf4cpp::rdf::Literal::make_typed_from_value<rdf4cpp::rdf::datatypes::xsd::Integer>(0),
							 boost::container::flat_set<rdf4cpp::rdf::Literal> rdf_literals = {});
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] SumDistinct *clone_impl() const override;
	};

	class Avg : public Aggregate {
	private:
		std::unique_ptr<SPARQLExpression> sum_expr_;
		std::unique_ptr<SPARQLExpression> count_expr_;
	public:
		explicit Avg(std::unique_ptr<SPARQLExpression> expr, bool distinct = false);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		Avg(std::unique_ptr<SPARQLExpression> expr, std::unique_ptr<SPARQLExpression> sum_expr, std::unique_ptr<SPARQLExpression> count_expr);
		[[nodiscard]] Avg *clone_impl() const override;
	};

	class Min : public Aggregate {
	private:
		rdf4cpp::rdf::Node rdf_node_;
	public:
		explicit Min(std::unique_ptr<SPARQLExpression> expr, rdf4cpp::rdf::Node rdf_node = {});
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Min *clone_impl() const override;
	};

	class Max : public Aggregate {
	private:
		rdf4cpp::rdf::Node rdf_node_;
	public:
		explicit Max(std::unique_ptr<SPARQLExpression> expr, rdf4cpp::rdf::Node rdf_node = {});
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Max *clone_impl() const override;
	};

	class Sample : public Aggregate {
	public:
		explicit Sample(std::unique_ptr<SPARQLExpression> expr);
		void update_value(detail::Key const &key) override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Sample *clone_impl() const override;
	};

}// namespace dice::sparql2tensor::expressions

#endif//TENTRIS_QUERY_SPARQL_AGGREGATES_HPP
