#include "SPARQLQuery.hpp"

namespace dice::sparql {

	SPARQLQuery::SPARQLQuery(std::shared_ptr<detail::HypertrieContext> tensor_context) : raw_query_{std::move(tensor_context)} {
	}

	detail::Query const &SPARQLQuery::raw_query() const { return raw_query_; }
	detail::Query &SPARQLQuery::raw_query() { return raw_query_; }

	SPARQLQuery::QueryType SPARQLQuery::query_type() const { return query_type_; }

	detail::OperandDependencyGraph const &SPARQLQuery::operand_dependency_graph() { return raw_query_.operand_dependency_graph(); };

	void SPARQLQuery::set_query_type(QueryType const &type) { query_type_ = type; }

	void SPARQLQuery::set_distinct() { raw_query_.set_distinct(); }

	void SPARQLQuery::set_aggregates() { raw_query_.set_aggregates(); }

	std::vector<rdf4cpp::rdf::query::TriplePattern> const &SPARQLQuery::construct_template() const { return construct_triple_templates_; }

	std::vector<rdf4cpp::rdf::query::QuadPattern> const &SPARQLQuery::delete_template() const { return delete_quad_templates_; }

	std::vector<rdf4cpp::rdf::query::QuadPattern> const &SPARQLQuery::insert_template() const { return insert_quad_templates_; }

	void SPARQLQuery::set_construct_template(std::vector<rdf4cpp::rdf::query::TriplePattern> triple_templates) {
		construct_triple_templates_ = std::move(triple_templates);
	}

	void SPARQLQuery::set_delete_template(std::vector<rdf4cpp::rdf::query::QuadPattern> quad_templates) {
		delete_quad_templates_ = std::move(quad_templates);
	}

	void SPARQLQuery::set_insert_template(std::vector<rdf4cpp::rdf::query::QuadPattern> quad_templates) {
		insert_quad_templates_ = std::move(quad_templates);
	}

	void SPARQLQuery::set_offset(ssize_t offset) {
		assert(offset >= 0);
		raw_query_.set_offset(offset);
	}

	void SPARQLQuery::set_limit(ssize_t limit) {
		assert(limit >= 0);
		raw_query_.set_limit(limit);
	}

	bool SPARQLQuery::contains_aggregates() const { return raw_query_.contains_aggregates(); }

	std::vector<rdf4cpp::rdf::query::Variable> const &SPARQLQuery::projected_variables() const { return projected_variables_; }

	void SPARQLQuery::set_operand_dependency_graph(detail::OperandDependencyGraph &&graph) {
		raw_query_.set_operand_dependency_graph(std::move(graph));
	}

	void SPARQLQuery::add_solution_binding(expressions::SPARQLExpressionWrapper expression) {
		raw_query_.add_binding(std::move(expression));
	}

	void SPARQLQuery::add_having_expression(expressions::SPARQLExpressionWrapper expression) {
		raw_query_.add_solution_filter(std::move(expression));
	}

	void SPARQLQuery::add_grouping_expression(expressions::SPARQLExpressionWrapper expression) {
		raw_query_.add_grouping_expression(std::move(expression));
	}

	void SPARQLQuery::add_ordering(expressions::SPARQLExpressionWrapper expression, bool desc) {
		raw_query_.add_ordering(std::move(expression), desc);
	}

	void SPARQLQuery::register_variable(rdf4cpp::rdf::query::Variable var) {
		if (variable_to_id_.contains(var))
			return;
		variable_to_id_[var] = next_variable_id_++;
	}

	char SPARQLQuery::variable_id(rdf4cpp::rdf::query::Variable variable) {
		assert(variable_to_id_.contains(variable));
		return variable_to_id_[variable];
	}

	void SPARQLQuery::track_variable(rdf4cpp::rdf::query::Variable variable) {
		assert(variable_to_id_.contains(variable));
		raw_query_.track_variable(variable_to_id_[variable]);
	}

	size_t SPARQLQuery::tracked_variable_position(rdf4cpp::rdf::query::Variable variable) const {
		assert(variable_to_id_.contains(variable));
		return raw_query_.tracked_var_position(variable_to_id_.find(variable)->second);
	}

	void SPARQLQuery::track_quad_template_variable(rdf4cpp::rdf::query::Variable variable) {
		if (not templates_variables_positions_.contains(variable)) {
			templates_variables_positions_[variable] = raw_query_.solution_mapping().size();
			// we need to create a binding in the raw query
			// by creating a binding the query returns the value of the variable,
			// it can be used to create the statements of CONSTRUCT, DELETE, and INSERT (similar to SELECT)
			raw_query_.add_binding(expressions::SPARQLExpressionWrapper{
					std::make_unique<expressions::PrimaryVarExpression>(variable, tracked_variable_position(variable))});
		}
	}

	size_t SPARQLQuery::quad_template_variable_position(rdf4cpp::rdf::query::Variable variable) const {
		assert(templates_variables_positions_.contains(variable));
		return templates_variables_positions_.find(variable)->second;
	}

	void SPARQLQuery::add_projected_variable(rdf4cpp::rdf::query::Variable variable) {
		projected_variables_.push_back(variable);
	}

	size_t SPARQLQuery::add_subquery(SPARQLQuery &&subquery) {
		return raw_query_.add_subquery(std::move(subquery.raw_query()));
	}

	size_t SPARQLQuery::add_inline_data(detail::BoolHypertrie &&inline_data) {
		return raw_query_.add_inline_data(std::move(inline_data));
	}

	std::shared_ptr<detail::HypertrieContext> SPARQLQuery::tensor_context() const noexcept {
		return raw_query_.tensor_context();
	}
}// namespace dice::sparql