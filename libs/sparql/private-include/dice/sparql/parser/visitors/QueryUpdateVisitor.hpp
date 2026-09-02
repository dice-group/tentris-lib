#ifndef TENTRIS_QUERY_SPARQL_QUERYUPDATEVISITOR_HPP
#define TENTRIS_QUERY_SPARQL_QUERYUPDATEVISITOR_HPP

#include <rdf4cpp/rdf/Node.hpp>
#include <rdf4cpp/rdf/query/Variable.hpp>
#include <SparqlParser/SparqlParserBaseVisitor.h>
#include <dice/sparql/SPARQLQuery.hpp>
#include <dice/sparql/expressions/expressions.hpp>

namespace dice::sparql::parser::visitors {


	class QueryUpdateVisitor : public dice::sparql_parser::base::SparqlParserBaseVisitor {
	private:
		// representation of GroupGraphPatternSub
		struct GroupGraphPatternSub {
			std::vector<std::variant<sparql_parser::base::SparqlParser::TriplesBlockContext *,
									 sparql_parser::base::SparqlParser::BindContext *>>
					triples_block_bind_ctxs;
			std::vector<sparql_parser::base::SparqlParser::GroupOrUnionGraphPatternContext *> group_or_union_ctxs;
			std::vector<sparql_parser::base::SparqlParser::FilterContext *> filter_ctxs;
			std::vector<sparql_parser::base::SparqlParser::OptionalGraphPatternContext *> optional_ctxs;
			std::vector<sparql_parser::base::SparqlParser::InlineDataContext *> inline_ctxs;
		};

		SPARQLQuery *const parsed_query_ptr_;
		robin_hood::unordered_map<std::string, std::string> const *const prefixes_ptr_;
		rdf4cpp::rdf::storage::node::NodeStorage const &node_storage_;
		rdf4cpp::rdf::Node active_graph_;
		rdf4cpp::rdf::Node active_subject_;
		rdf4cpp::rdf::Node active_object_;
		rdf4cpp::rdf::Node active_predicate_;
		std::vector<rdf4cpp::rdf::query::QuadPattern> quad_patterns_;
		std::unordered_set<rdf4cpp::rdf::query::Variable> vars_in_scope_;
		std::unordered_set<rdf4cpp::rdf::query::Variable> vars_in_group_by_;
		std::unordered_set<rdf4cpp::rdf::query::Variable> non_agg_vars_in_select_;
		std::unordered_set<rdf4cpp::rdf::query::Variable> non_agg_vars_in_having_;
		std::unordered_set<rdf4cpp::rdf::query::Variable> non_agg_vars_in_order_by_;
		std::unordered_set<rdf4cpp::rdf::query::Variable> current_scope_;
		mutable boost::container::flat_map<SPARQLQuery::VarID , SPARQLQuery::VarID> equality_filter_var_replacement_;
		robin_hood::unordered_map<rdf4cpp::rdf::query::Variable, expressions::SPARQLExpressionWrapper> aliases_;
		bool in_filter_ = false;// used to check if filter variables are in-scope
		bool in_bind_ = false;// used to check if variables of bind expressions are  in-bgp-scope
		bool has_grouping_ = false;
		size_t bnode_anon_counter_ = 0;
		bool in_delete_template_ = false;
		bool in_update_data_ = false;

	public:
		// constructors
		QueryUpdateVisitor(SPARQLQuery &parsed_query,
						   robin_hood::unordered_map<std::string, std::string> const &prefixes,
						   rdf4cpp::rdf::storage::node::NodeStorage const &node_storage);
		// visit functions (overloaded)
		std::any visitAskQuery(sparql_parser::base::SparqlParser::AskQueryContext *ctx) override;
		std::any visitConstructQuery(sparql_parser::base::SparqlParser::ConstructQueryContext *ctx) override;
		std::any visitConstructTemplate(sparql_parser::base::SparqlParser::ConstructTemplateContext *ctx) override;
		std::any visitUpdate(sparql_parser::base::SparqlParser::UpdateContext *ctx) override;
		std::any visitModify(sparql_parser::base::SparqlParser::ModifyContext *ctx) override;
		std::any visitDeleteWhere(sparql_parser::base::SparqlParser::DeleteWhereContext *ctx) override;
		std::any visitInsertData(sparql_parser::base::SparqlParser::InsertDataContext *ctx) override;
		std::any visitDeleteData(sparql_parser::base::SparqlParser::DeleteDataContext *ctx) override;
		std::any visitSelectQuery(sparql_parser::base::SparqlParser::SelectQueryContext *ctx) override;
		std::any visitSelectClause(sparql_parser::base::SparqlParser::SelectClauseContext *ctx) override;
		std::any visitWhereClause(sparql_parser::base::SparqlParser::WhereClauseContext *ctx) override;
		std::any visitQuads(sparql_parser::base::SparqlParser::QuadsContext *ctx) override;
		std::any visitTriplesSameSubject(sparql_parser::base::SparqlParser::TriplesSameSubjectContext *ctx) override;
		std::any visitPropertyListNotEmpty(sparql_parser::base::SparqlParser::PropertyListNotEmptyContext *ctx) override;
		std::any visitGroupClause(sparql_parser::base::SparqlParser::GroupClauseContext *ctx) override;
		std::any visitHavingClause(sparql_parser::base::SparqlParser::HavingClauseContext *ctx) override;
		std::any visitOrderClause(sparql_parser::base::SparqlParser::OrderClauseContext *ctx) override;
		std::any visitLimitOffsetClauses(sparql_parser::base::SparqlParser::LimitOffsetClausesContext *ctx) override;
		std::any visitExpression(sparql_parser::base::SparqlParser::ExpressionContext *ctx);
		std::any visitPrimaryExpression(sparql_parser::base::SparqlParser::PrimaryExpressionContext *ctx) override;
		std::any visitConditionalAndExpression(sparql_parser::base::SparqlParser::ConditionalAndExpressionContext *ctx) override;
		std::any visitConditionalOrExpression(sparql_parser::base::SparqlParser::ConditionalOrExpressionContext *ctx) override;
		std::any visitAdditiveExpression(sparql_parser::base::SparqlParser::AdditiveExpressionContext *ctx) override;
		std::any visitUnaryAdditiveExpression(sparql_parser::base::SparqlParser::UnaryAdditiveExpressionContext *ctx) override;
		std::any visitMultiplicativeExpression(sparql_parser::base::SparqlParser::MultiplicativeExpressionContext *ctx) override;
		std::any visitRelationalExpression(sparql_parser::base::SparqlParser::RelationalExpressionContext *ctx) override;
		std::any visitRelationalSetExpression(sparql_parser::base::SparqlParser::RelationalSetExpressionContext *ctx) override;
		std::any visitUnaryNegationExpression(sparql_parser::base::SparqlParser::UnaryNegationExpressionContext *ctx) override;
		std::any visitUnarySignedLiteralExpression(sparql_parser::base::SparqlParser::UnarySignedLiteralExpressionContext *ctx) override;
		std::any visitUnaryLiteralExpression(sparql_parser::base::SparqlParser::UnaryLiteralExpressionContext *ctx) override;
		std::any visitBuiltInCall(sparql_parser::base::SparqlParser::BuiltInCallContext *ctx) override;
		std::any visitFunctionCall(sparql_parser::base::SparqlParser::FunctionCallContext *ctx) override;
		std::any visitAggregate(sparql_parser::base::SparqlParser::AggregateContext *ctx) override;
		std::any visitVarOrTerm(sparql_parser::base::SparqlParser::VarOrTermContext *ctx) override;
		std::any visitVar(sparql_parser::base::SparqlParser::VarContext *ctx) override;
		std::any visitRdfLiteral(sparql_parser::base::SparqlParser::RdfLiteralContext *ctx) override;
		std::any visitNumericLiteral(sparql_parser::base::SparqlParser::NumericLiteralContext *ctx) override;
		std::any visitNumericLiteralUnsigned(sparql_parser::base::SparqlParser::NumericLiteralUnsignedContext *ctx) override;
		std::any visitNumericLiteralPositive(sparql_parser::base::SparqlParser::NumericLiteralPositiveContext *ctx) override;
		std::any visitNumericLiteralNegative(sparql_parser::base::SparqlParser::NumericLiteralNegativeContext *ctx) override;
		std::any visitBooleanLiteral(sparql_parser::base::SparqlParser::BooleanLiteralContext *ctx) override;
		std::any visitString(sparql_parser::base::SparqlParser::StringContext *ctx) override;
		std::any visitIri(sparql_parser::base::SparqlParser::IriContext *ctx) override;
		std::any visitBlankNode(sparql_parser::base::SparqlParser::BlankNodeContext *ctx) override;

		// visit functions (non-overloaded)
		detail::OperandDependencyGraph visit_sub_select(sparql_parser::base::SparqlParser::SubSelectContext *ctx);
		detail::OperandDependencyGraph visit_group_graph_pattern(sparql_parser::base::SparqlParser::GroupGraphPatternContext *ctx);
		detail::OperandDependencyGraph visit_group_graph_pattern_sub(sparql_parser::base::SparqlParser::GroupGraphPatternSubContext *ctx);
		detail::OperandDependencyGraph visit_group_or_union_graph_pattern(sparql_parser::base::SparqlParser::GroupOrUnionGraphPatternContext *ctx);
		detail::OperandDependencyGraph visit_optional_graph_pattern(sparql_parser::base::SparqlParser::OptionalGraphPatternContext *ctx);
		detail::OperandDependencyGraph visit_filter(sparql_parser::base::SparqlParser::FilterContext *ctx);
		detail::OperandDependencyGraph visit_bind(sparql_parser::base::SparqlParser::BindContext *ctx);
		detail::OperandDependencyGraph visit_triples_block(sparql_parser::base::SparqlParser::TriplesBlockContext *ctx);
		detail::OperandDependencyGraph visit_triples_same_subject_path(sparql_parser::base::SparqlParser::TriplesSameSubjectPathContext *ctx);
		detail::OperandDependencyGraph visit_property_list_path_not_empty(sparql_parser::base::SparqlParser::PropertyListPathNotEmptyContext *ctx);
		detail::OperandDependencyGraph visit_blank_node_property_list_path(sparql_parser::base::SparqlParser::BlankNodePropertyListPathContext *ctx);
		detail::OperandDependencyGraph visit_verb_simple(sparql_parser::base::SparqlParser::VerbSimpleContext *ctx);
		detail::OperandDependencyGraph visit_path(sparql_parser::base::SparqlParser::PathContext *ctx);
		detail::OperandDependencyGraph visit_path_alternative(sparql_parser::base::SparqlParser::PathAlternativeContext *ctx);
		detail::OperandDependencyGraph visit_path_sequence(sparql_parser::base::SparqlParser::PathSequenceContext *ctx);
		detail::OperandDependencyGraph visit_path_elt(sparql_parser::base::SparqlParser::PathEltContext *ctx);
		detail::OperandDependencyGraph visit_path_elt_or_inverse(sparql_parser::base::SparqlParser::PathEltOrInverseContext *ctx);
		detail::OperandDependencyGraph visit_inline_data(sparql_parser::base::SparqlParser::InlineDataContext *ctx);
		detail::OperandDependencyGraph visit_inline_data_one_var(sparql_parser::base::SparqlParser::InlineDataOneVarContext *ctx);
		detail::OperandDependencyGraph visit_inline_data_full(sparql_parser::base::SparqlParser::InlineDataFullContext *ctx);

	private:
		template<typename T>
		std::any visitSelectQuery_impl(T *ctx);

		template<typename T>
		std::any visitFunctionCall_impl(T *ctx);

		template<typename T>
		expressions::SPARQLExpressionWrapper visitXSDFunction_impl(std::string_view const &identifier, T *ctx);

		/**
		 * @brief Creates a variable from the provided blank node label. Takes care of registering the variable.
		 */
		rdf4cpp::rdf::query::Variable variable_from_blank_node(std::string const &blank_node_label) const noexcept;

		/**
		 * @brief Check if the provided variable is an alias of another expression. If it is not, a new PrimaryVarExpression is created.
		 */
		expressions::SPARQLExpressionWrapper check_for_alias(rdf4cpp::rdf::query::Variable variable) const noexcept;

		/**
		 * @brief Dedicated visitor for SPARQL EXISTS
		 */
		expressions::SPARQLExpressionWrapper visitExists(sparql_parser::base::SparqlParser::GroupGraphPatternContext *ctx, bool is_not = false);

		/**
		 * @brief Dedicated visitor for XSD functions
		 */
		expressions::SPARQLExpressionWrapper visitXSDFunction(std::string_view const &identifier, sparql_parser::base::SparqlParser::FunctionCallContext *ctx);

		/**
		 * @brief Creates a query operand from the active triple pattern in the provided operand dependency graph
		 * The triple pattern is implicitly defined by the active_subject_, active_predicate_ and active_object_ fields.
		 * It also adds the variables to the scope of group graph patterns and the query
		 */
		detail::operand_desc create_operand_from_tp(detail::OperandDependencyGraph &odg);

		/**
		 * @brief Creates a query operand from a filter expression.
		 */
		std::optional<detail::operand_desc> create_operand_from_filter(detail::OperandDependencyGraph &odg, std::unique_ptr<expressions::SPARQLExpression> expression) const;

		/**
		 * @brief Creates bidirectional dependencies between the provided descriptors within the provided graph
		 */
		void create_dependencies_between_operands(detail::OperandDependencyGraph &odg, std::vector<detail::operand_desc> const &descriptors) const;

		/**
		 * @brief Rewrites to UNION normal form by combining the different union components and merging them
		 */
		detail::OperandDependencyGraph combine_and_merge(detail::OperandDependencyGraph &first_odg, detail::OperandDependencyGraph &second_odg, bool bidirectional = true) const;

		/**
		 * @brief Combines non-optional and optional graphs. Does not affect UNION patterns found within OPTIONAL
		 */
		detail::OperandDependencyGraph combine_optional(detail::OperandDependencyGraph &first_odg, detail::OperandDependencyGraph &second_odg) const;

		/**
		 * @brief Captures Cartesian products between optional group graph patterns
		 */
		detail::OperandDependencyGraph optional_cartesian_connections(detail::OperandDependencyGraph &first_odg, detail::OperandDependencyGraph &second_odg) const;

		/**
		 * @brief Creates an operand dependency graph from the provided GroupGraphPatternSub
		 */
		detail::OperandDependencyGraph group_graph_pattern_sub_to_odg(GroupGraphPatternSub const &group_graph_pattern_sub);

		/**
		 * @brief Tries to find the iri in node_storage_. If it is not found, the iri is created (retrieved) in (from) default_storage
		 */
		rdf4cpp::rdf::IRI try_get_in_node_storage_else_default(rdf4cpp::rdf::IRI iri, rdf4cpp::rdf::storage::node::NodeStorage &default_storage = rdf4cpp::rdf::storage::node::NodeStorage::default_instance()) const;

		/**
		 * @brief Tries to find the literal in node_storage_. If it is not found, the literal is created (retrieved) in (from) default_storage
		 */
		rdf4cpp::rdf::Literal try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal literal, rdf4cpp::rdf::storage::node::NodeStorage &default_storage = rdf4cpp::rdf::storage::node::NodeStorage::default_instance()) const;
	};

}// namespace dice::sparql::parser::visitors

#endif//TENTRIS_QUERY_SPARQL_QUERYUPDATEVISITOR_HPP
