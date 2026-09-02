#ifndef TENTRIS_QUERY_SPARQL_PARSEDSPARQL_HPP
#define TENTRIS_QUERY_SPARQL_PARSEDSPARQL_HPP

#include "detail/tensor.hpp"
#include "expressions/expressions.hpp"

#include <rdf4cpp/rdf/query/Variable.hpp>
#include <rdf4cpp/rdf/query/TriplePattern.hpp>

namespace dice::sparql {

	/**
	 * @brief Represents a parsed SPARQL query
	 */
	class SPARQLQuery {
	public:
		using VarID = char;
		enum class QueryType {
			SELECT            = 0,
			ASK               = 1,
			CONSTRUCT         = 2,
			DESCRIBE          = 3,
			DELETE            = 4,
			INSERT            = 5,
			DELETE_AND_INSERT = 6,
			DELETE_DATA       = 7,
			INSERT_DATA       = 8
		};

	private:
		// the raw query object
		detail::Query raw_query_;
		// the type of the query
		QueryType query_type_;
		// the variables of the quads patterns will be bound to the bindings returned by the query
		// used by CONSTRUCT (construct needs only triple patterns)
		std::vector<rdf4cpp::rdf::query::TriplePattern> construct_triple_templates_;
		// used by DELETE operations
		std::vector<rdf4cpp::rdf::query::QuadPattern> delete_quad_templates_;
		// used by INSERT operations
		std::vector<rdf4cpp::rdf::query::QuadPattern> insert_quad_templates_;
		// used by CONSTRUCT, DELETE, and INSERT. keeps track of the positions of variables in the solution returned by the raw_query
		// the positions are used to access the binding of each variable. the binding is turn used to instantiate the triple of a particular operation
		boost::container::flat_map<rdf4cpp::rdf::query::Variable, size_t> templates_variables_positions_;
		// only for SELECT
		std::vector<rdf4cpp::rdf::query::Variable> projected_variables_;
		// maps variables to ids
		using VariableHash = dice::hash::DiceHashxxh3<rdf4cpp::rdf::query::Variable>;
		robin_hood::unordered_map<rdf4cpp::rdf::query::Variable, VarID, VariableHash> variable_to_id_;
		char next_variable_id_ = 'a';

	public:
		/* constructors */
		explicit SPARQLQuery(std::shared_ptr<detail::HypertrieContext> tensor_context);

		/* read functions */
		QueryType query_type() const;
		detail::OperandDependencyGraph const &operand_dependency_graph();
		[[nodiscard]] bool contains_aggregates() const;
		[[nodiscard]] VarID variable_id(rdf4cpp::rdf::query::Variable variable);
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> const &projected_variables() const;
		[[nodiscard]] detail::Query const &raw_query() const;
		[[nodiscard]] detail::Query &raw_query();
		[[nodiscard]] size_t tracked_variable_position(rdf4cpp::rdf::query::Variable variable) const;
		[[nodiscard]] size_t quad_template_variable_position(rdf4cpp::rdf::query::Variable variable) const;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::TriplePattern> const &construct_template() const;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::QuadPattern> const &delete_template() const;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::QuadPattern> const &insert_template() const;

		/* write functions */
		void set_query_type(QueryType const &type);
		void set_distinct();
		void set_offset(ssize_t offset);
		void set_limit(ssize_t limit);
		void set_aggregates();
		void set_construct_template(std::vector<rdf4cpp::rdf::query::TriplePattern> triple_templates);
		void set_delete_template(std::vector<rdf4cpp::rdf::query::QuadPattern> quad_templates);
		void set_insert_template(std::vector<rdf4cpp::rdf::query::QuadPattern> quad_templates);
		void set_operand_dependency_graph(detail::OperandDependencyGraph &&graph);
		void add_solution_binding(expressions::SPARQLExpressionWrapper expression);
		void add_having_expression(expressions::SPARQLExpressionWrapper expression);
		void add_grouping_expression(expressions::SPARQLExpressionWrapper expression);
		void add_ordering(expressions::SPARQLExpressionWrapper expression, bool desc);
		void register_variable(rdf4cpp::rdf::query::Variable var); // assigns an id to the provided variable
		void track_variable(rdf4cpp::rdf::query::Variable variable); // tracked variables will be part of entry in evaluation
		// tracks the variables that appear in the templates of CONSTRUCT, DELETE and INSERT by creating a new binding in the raw query
		// and storing the position of the variable in the binding (templates_variables_positions_)
		void track_quad_template_variable(rdf4cpp::rdf::query::Variable variable);
		void add_projected_variable(rdf4cpp::rdf::query::Variable variable);
		size_t add_subquery(SPARQLQuery &&subquery);
		size_t add_inline_data(detail::BoolHypertrie &&inline_data);

		/**
		 * @return the hypertrie context that should be used for this query
		 * @warning the context is not synchronized in any way and thus cannot be used in parallel
		 * @todo when subquery processing is parallelized care must be taken to give subqueries their own context, or lock this one
		 */
		std::shared_ptr<detail::HypertrieContext> tensor_context() const noexcept;
	};

}

#endif  //TENTRIS_QUERY_SPARQL_PARSEDSPARQL_HPP
