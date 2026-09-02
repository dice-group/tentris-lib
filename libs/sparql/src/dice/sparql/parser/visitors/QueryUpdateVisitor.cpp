#include "dice/sparql/parser/visitors/QueryUpdateVisitor.hpp"
#include "dice/sparql/parser/exception/Exceptions.hpp"

#define UNSUPPORTED_OP_EXCEPTION(ctx, op) exception::unsupported_query{ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine(), op};
#define MALFORMED_QUERY_EXCEPTION(ctx, op) exception::malformed_query{ctx->getStart()->getLine(), ctx->getStart()->getCharPositionInLine(), op};

namespace dice::sparql::parser::visitors {

	using namespace dice::sparql::expressions;

	QueryUpdateVisitor::QueryUpdateVisitor(SPARQLQuery &parsed_query,
										   robin_hood::unordered_map<std::string, std::string> const &prefixes,
										   rdf4cpp::rdf::storage::node::NodeStorage const &node_storage)
		: parsed_query_ptr_(&parsed_query), prefixes_ptr_(&prefixes), node_storage_(node_storage) {}

	std::any QueryUpdateVisitor::visitAskQuery(sparql_parser::base::SparqlParser::AskQueryContext *ctx) {
		parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::ASK);
		visitWhereClause(ctx->whereClause());
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitConstructQuery(sparql_parser::base::SparqlParser::ConstructQueryContext *ctx) {
		parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::CONSTRUCT);
		if (not ctx->datasetClause().empty())
			throw UNSUPPORTED_OP_EXCEPTION(ctx, "DatasetClause");
		// case: Construct template + WHERE clause
		if (auto construct_template_ctx = ctx->constructTemplate()) {
			visitWhereClause(ctx->whereClause());
			auto solution_modifiers_ctx = ctx->solutionModifier();
			if (auto group_clause_ctx = solution_modifiers_ctx->groupClause(); group_clause_ctx)
				throw MALFORMED_QUERY_EXCEPTION(ctx, "GROUP BY in CONSTRUCT query");
			if (auto having_clause_ctx = solution_modifiers_ctx->havingClause(); having_clause_ctx)
				visitHavingClause(having_clause_ctx);
			visitConstructTemplate(ctx->constructTemplate());
			// limit and ordering should be processed after the templated triples
			if (auto limit_offset_ctx = solution_modifiers_ctx->limitOffsetClauses(); limit_offset_ctx)
				visitLimitOffsetClauses(limit_offset_ctx);
			if (auto order_by_ctx = solution_modifiers_ctx->orderClause(); order_by_ctx)
				visitOrderClause(order_by_ctx);
		}
		// case: short-handed version CONSTRUCT WHERE {}
		// the WHERE clause is also used as the CONSTRUCT template
		// info: https://www.w3.org/TR/2013/REC-sparql11-query-20130321/#constructWhere
		else {
			for (auto triples_same_subject_ctx : ctx->triplesTemplate()->triplesSameSubject()) {
				visitTriplesSameSubject(triples_same_subject_ctx);
			}
			if (auto having_clause_ctx = ctx->solutionModifier()->havingClause(); having_clause_ctx)
				visitHavingClause(having_clause_ctx);
			// limit and ordering should be processed after the templated triples
			if (auto limit_offset_ctx = ctx->solutionModifier()->limitOffsetClauses(); limit_offset_ctx)
				visitLimitOffsetClauses(limit_offset_ctx);
			if (auto order_by_ctx = ctx->solutionModifier()->orderClause(); order_by_ctx)
				visitOrderClause(order_by_ctx);
			// create the operand dependency graph
			detail::OperandDependencyGraph operand_dependency_graph{};
			std::vector<detail::operand_desc> descriptors{};
			// create a vertex in the graph for each triple pattern
			// create_operand_from_tp uses active_subject_, active_predicate_, and active_object_
			// we have to manually assign their value here
			std::vector<rdf4cpp::rdf::query::TriplePattern> triple_templates{};
			for (auto const &quad_pattern : quad_patterns_) {
				// graph is not used in construct
				active_subject_ = quad_pattern.subject();
				active_predicate_ = quad_pattern.predicate();
				active_object_ = quad_pattern.object();
				descriptors.push_back(create_operand_from_tp(operand_dependency_graph));
				triple_templates.emplace_back(active_subject_, active_predicate_, active_object_);
			}
			create_dependencies_between_operands(operand_dependency_graph, descriptors);
			parsed_query_ptr_->set_construct_template(std::move(triple_templates));
			parsed_query_ptr_->set_operand_dependency_graph(std::move(operand_dependency_graph));
			quad_patterns_.clear();
		}
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitConstructTemplate(sparql_parser::base::SparqlParser::ConstructTemplateContext *ctx) {
		if (auto triples_same_subject_ctx = ctx->constructTriples()->triplesSameSubject(); triples_same_subject_ctx)
			visitTriplesSameSubject(triples_same_subject_ctx);
		for (auto construct_triple_ctx : ctx->constructTriples()->constructTriples()) {
			visitTriplesSameSubject(construct_triple_ctx->triplesSameSubject());
		}
		// create triples from quads
		std::vector<rdf4cpp::rdf::query::TriplePattern> triple_templates{};
		for (auto const &quad_pattern : quad_patterns_) {
			triple_templates.emplace_back(quad_pattern.subject(), quad_pattern.predicate(), quad_pattern.object());
		}
		parsed_query_ptr_->set_construct_template(std::move(triple_templates));
		quad_patterns_.clear();
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitUpdate(sparql_parser::base::SparqlParser::UpdateContext *ctx) {
		if (auto delete_where_ctx = ctx->deleteWhere(); delete_where_ctx)
			return visitDeleteWhere(delete_where_ctx);
		if (auto modify_ctx = ctx->modify(); modify_ctx)
			return visitModify(modify_ctx);
		if (auto delete_data_ctx = ctx->deleteData(); delete_data_ctx)
			return visitDeleteData(delete_data_ctx);
		if (auto insert_data_ctx = ctx->insertData(); insert_data_ctx)
			return visitInsertData(insert_data_ctx);
		throw UNSUPPORTED_OP_EXCEPTION(ctx, "Unsupported update operation");
	}

	std::any QueryUpdateVisitor::visitInsertData(sparql_parser::base::SparqlParser::InsertDataContext *ctx) {
		parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::INSERT_DATA);
		in_update_data_ = true;
		visitQuads(ctx->quadData()->quads());
		in_update_data_ = false;
		parsed_query_ptr_->set_insert_template(std::move(quad_patterns_));
		quad_patterns_.clear();
		return nullptr;
	}
	std::any QueryUpdateVisitor::visitDeleteData(sparql_parser::base::SparqlParser::DeleteDataContext *ctx) {
		parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::DELETE_DATA);
		in_update_data_ = true;
		in_delete_template_ = true;
		visitQuads(ctx->quadData()->quads());
		in_delete_template_ = false;
		in_update_data_ = false;
		parsed_query_ptr_->set_delete_template(std::move(quad_patterns_));
		quad_patterns_.clear();
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitModify(sparql_parser::base::SparqlParser::ModifyContext *ctx) {
		if (ctx->WITH() or not ctx->usingClause().empty())
			throw UNSUPPORTED_OP_EXCEPTION(ctx, "Named graphs not supported in update operations");
		auto operand_dependency_graph = (visit_group_graph_pattern(ctx->groupGraphPattern()));
		parsed_query_ptr_->set_operand_dependency_graph(std::move(operand_dependency_graph));
		if (ctx->deleteClause()) {
			in_delete_template_ = true;
			visitQuads(ctx->deleteClause()->quadPattern()->quads());
			in_delete_template_ = false;
			parsed_query_ptr_->set_delete_template(std::move(quad_patterns_));
			quad_patterns_.clear();
			parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::DELETE);
		}
		if (ctx->insertClause()) {
			visitQuads(ctx->insertClause()->quadPattern()->quads());
			parsed_query_ptr_->set_insert_template(std::move(quad_patterns_));
			quad_patterns_.clear();
			if (ctx->deleteClause())
				parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::DELETE_AND_INSERT);
			else
				parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::INSERT);
		}
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitDeleteWhere(sparql_parser::base::SparqlParser::DeleteWhereContext *ctx) {
		parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::DELETE);
		in_delete_template_ = true;
		visitQuads(ctx->quadPattern()->quads());
		in_delete_template_ = false;
		// create the operand dependency graph
		detail::OperandDependencyGraph operand_dependency_graph{};
		std::vector<detail::operand_desc> descriptors{};
		// create a vertex in the graph for each triple pattern
		// create_operand_from_tp uses active_subject_, active_predicate_, and active_object_
		// we have to manually assign their value here
		for (auto const &quad_pattern : quad_patterns_) {
			active_graph_ = quad_pattern.graph(); // currently not used (GRAPH keyword not supported)
			active_subject_ = quad_pattern.subject();
			active_predicate_ = quad_pattern.predicate();
			active_object_ = quad_pattern.object();
			descriptors.push_back(create_operand_from_tp(operand_dependency_graph));
		}
		create_dependencies_between_operands(operand_dependency_graph, descriptors);
		parsed_query_ptr_->set_delete_template(std::move(quad_patterns_));
		parsed_query_ptr_->set_operand_dependency_graph(std::move(operand_dependency_graph));
		quad_patterns_.clear();
		return nullptr;
	}

	template<typename T>
	std::any QueryUpdateVisitor::visitSelectQuery_impl(T *ctx) {
		parsed_query_ptr_->set_query_type(SPARQLQuery::QueryType::SELECT);

		if constexpr (requires { ctx->datasetClause(); }) {
			if (not ctx->datasetClause().empty())
				throw UNSUPPORTED_OP_EXCEPTION(ctx, "DatasetClause");
		}

		visitWhereClause(ctx->whereClause());
		auto solution_modifiers_ctx = ctx->solutionModifier();
		// group by and having should be parsed before the select clause
		if (auto group_clause_ctx = solution_modifiers_ctx->groupClause(); group_clause_ctx)
			visitGroupClause(group_clause_ctx);
		if (auto having_clause_ctx = solution_modifiers_ctx->havingClause(); having_clause_ctx)
			visitHavingClause(having_clause_ctx);
		visitSelectClause(ctx->selectClause());
		// limit and ordering should be processed after the select clause
		if (auto limit_offset_ctx = solution_modifiers_ctx->limitOffsetClauses(); limit_offset_ctx)
			visitLimitOffsetClauses(limit_offset_ctx);
		if (auto order_by_ctx = solution_modifiers_ctx->orderClause(); order_by_ctx)
			visitOrderClause(order_by_ctx);
		// check for usage of non-aggregated and non-grouped variables
		if (parsed_query_ptr_->contains_aggregates()) {
			for (auto const &var : non_agg_vars_in_having_) {
				if (not vars_in_group_by_.contains(var))
					throw MALFORMED_QUERY_EXCEPTION(ctx,
													"Non-aggregated variable " + var.backend_handle().variable_backend().n_string() +
															" is not part of the group key");
			}
			for (auto const &var : non_agg_vars_in_select_) {
				if (not vars_in_group_by_.contains(var))
					throw MALFORMED_QUERY_EXCEPTION(ctx,
													"Non-aggregated variable " + var.backend_handle().variable_backend().n_string() +
															" is not part of the group key");
			}
			for (auto const &var : non_agg_vars_in_order_by_) {
				if (not vars_in_group_by_.contains(var))
					throw MALFORMED_QUERY_EXCEPTION(ctx,
													"Non-aggregated variable " + var.backend_handle().variable_backend().n_string() +
															" is not part of the group key");
			}
		}
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitSelectQuery(sparql_parser::base::SparqlParser::SelectQueryContext *ctx) {
		return visitSelectQuery_impl(ctx);
	}

	std::any QueryUpdateVisitor::visitSelectClause(sparql_parser::base::SparqlParser::SelectClauseContext *ctx) {
		if (ctx->selectModifier() and ctx->selectModifier()->DISTINCT())
			parsed_query_ptr_->set_distinct();
		// case: project all variables
		if (ctx->ASTERISK()) {
			if (has_grouping_)
				throw MALFORMED_QUERY_EXCEPTION(ctx, "GROUP BY is not allowed in SELECT * queries");
			for (auto const &var : vars_in_scope_) {
				if (not var.is_anonymous()) {
					parsed_query_ptr_->add_projected_variable(var);
					parsed_query_ptr_->track_variable(var);
					parsed_query_ptr_->add_solution_binding(
							SPARQLExpressionWrapper{std::make_unique<PrimaryVarExpression>(var, parsed_query_ptr_->tracked_variable_position(var))});
				}
			}
		}
		// case: project variables or expressions
		else {
			for (auto sel_ctx : ctx->selectVariables()) {
				auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(sel_ctx->var()));
				// the same variable should not be projected multiple times
				if (std::find(parsed_query_ptr_->projected_variables().begin(), parsed_query_ptr_->projected_variables().end(), var) !=
					parsed_query_ptr_->projected_variables().end()) {
					throw MALFORMED_QUERY_EXCEPTION(ctx, "Variable " + var.backend_handle().variable_backend().n_string() + " is already projected.");
				}
				parsed_query_ptr_->add_projected_variable(var);
				parsed_query_ptr_->register_variable(var);
				// AS expressions should not use variables that are already in scope
				if (sel_ctx->AS()) {
					if (vars_in_scope_.contains(var))
						throw MALFORMED_QUERY_EXCEPTION(ctx, "Variable " + var.backend_handle().variable_backend().n_string() + " is already in scope.");
					auto expression = std::any_cast<SPARQLExpressionWrapper>(visitExpression(sel_ctx->expression()));
					for (auto non_agg_var : expression.non_aggregated_variables()) {
						non_agg_vars_in_select_.insert(non_agg_var);
					}
					aliases_.emplace(var, expression);
					parsed_query_ptr_->add_solution_binding(std::move(expression));
				}
				// the ids of projected variables (not of AS expressions) need to be passed to the query library
				else {
					non_agg_vars_in_select_.insert(var);
					parsed_query_ptr_->add_solution_binding(check_for_alias(var));
				}
				vars_in_scope_.insert(var);
			}
			if (parsed_query_ptr_->projected_variables().empty()) {
				throw MALFORMED_QUERY_EXCEPTION(ctx, "At least one variable should be projected.");
			}
		}
		return nullptr;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_sub_select(sparql_parser::base::SparqlParser::SubSelectContext *ctx) {
		SPARQLQuery parsed_subquery{parsed_query_ptr_->tensor_context()};
		QueryUpdateVisitor subquery_visitor{parsed_subquery, *prefixes_ptr_, node_storage_};
		subquery_visitor.visitSelectQuery_impl(ctx);
		std::vector<SPARQLQuery::VarID> var_ids{};
		if (parsed_subquery.projected_variables().size() > 3)
			throw UNSUPPORTED_OP_EXCEPTION(ctx, "Subqueries support up to three projected variables.");
		for (auto const &var : parsed_subquery.projected_variables()) {
			parsed_query_ptr_->register_variable(var);
			current_scope_.insert(var);// the vars are not part of the scope
			vars_in_scope_.insert(var);
			var_ids.push_back(parsed_query_ptr_->variable_id(var));
		}
		auto id = parsed_query_ptr_->add_subquery(std::move(parsed_subquery));
		detail::OperandDependencyGraph operand_dependency_graph{};
		operand_dependency_graph.add_subquery(var_ids, id);
		if (auto values_data_ctx = ctx->valuesClause()->dataBlock(); values_data_ctx) {
			if (auto one_var_ctx = values_data_ctx->inlineDataOneVar(); one_var_ctx) {
				auto temp_graph = (visit_inline_data_one_var(one_var_ctx));
				operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
			} else {
				auto temp_graph = (visit_inline_data_full(values_data_ctx->inlineDataFull()));
				operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
			}
		}
		return operand_dependency_graph;
	}

	std::any QueryUpdateVisitor::visitWhereClause(sparql_parser::base::SparqlParser::WhereClauseContext *ctx) {
		auto operand_dependency_graph = (visit_group_graph_pattern(ctx->groupGraphPattern()));
		parsed_query_ptr_->set_operand_dependency_graph(std::move(operand_dependency_graph));
		return nullptr;
	}

	// returns a single operand dependency graph (subqueries always return graphs consisting of a single vertex)
	detail::OperandDependencyGraph QueryUpdateVisitor::visit_group_graph_pattern(sparql_parser::base::SparqlParser::GroupGraphPatternContext *ctx) {
		detail::OperandDependencyGraph operand_dependency_graph{};
		if (auto sub_select_ctx = ctx->subSelect(); sub_select_ctx)
			operand_dependency_graph = (visit_sub_select(sub_select_ctx));
		else
			operand_dependency_graph = (visit_group_graph_pattern_sub(ctx->groupGraphPatternSub()));
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_group_graph_pattern_sub(sparql_parser::base::SparqlParser::GroupGraphPatternSubContext *ctx) {
		detail::OperandDependencyGraph operand_dependency_graph{};
		GroupGraphPatternSub group_graph_pattern_sub{};
		if (auto triples_block_ctx = ctx->triplesBlock(); triples_block_ctx)
			group_graph_pattern_sub.triples_block_bind_ctxs.push_back(triples_block_ctx);
		for (auto sub_list_ctx : ctx->groupGraphPatternSubList()) {
			if (auto not_triples_ctx = sub_list_ctx->graphPatternNotTriples(); not_triples_ctx) {
				if (auto union_ctx = not_triples_ctx->groupOrUnionGraphPattern(); union_ctx) {
					group_graph_pattern_sub.group_or_union_ctxs.push_back(union_ctx);
				} else if (auto opt_ctx = not_triples_ctx->optionalGraphPattern(); opt_ctx) {
					group_graph_pattern_sub.optional_ctxs.push_back(opt_ctx);
				} else if (auto filter_ctx = not_triples_ctx->filter(); filter_ctx) {
					group_graph_pattern_sub.filter_ctxs.push_back(filter_ctx);
				} else if (auto inline_ctx = not_triples_ctx->inlineData(); inline_ctx) {
					group_graph_pattern_sub.inline_ctxs.push_back(inline_ctx);
				} else if (auto bind_ctx = not_triples_ctx->bind(); bind_ctx) {
					group_graph_pattern_sub.triples_block_bind_ctxs.push_back(bind_ctx);
				} else if (not_triples_ctx->minusGraphPattern()) {
					throw UNSUPPORTED_OP_EXCEPTION(ctx, "MINUS");
				} else if (not_triples_ctx->serviceGraphPattern()) {
					throw UNSUPPORTED_OP_EXCEPTION(ctx, "SERVICE");
				} else if (not_triples_ctx->graphGraphPattern()) {
					throw UNSUPPORTED_OP_EXCEPTION(ctx, "GRAPH");
				} else {
					assert(false);
				}
			}
			if (auto triples_block_ctx = sub_list_ctx->triplesBlock()) {
				group_graph_pattern_sub.triples_block_bind_ctxs.push_back(triples_block_ctx);
			}
		}
		auto temp_graph = group_graph_pattern_sub_to_odg(group_graph_pattern_sub);
		return combine_and_merge(operand_dependency_graph, temp_graph);
	}

	std::any QueryUpdateVisitor::visitQuads(sparql_parser::base::SparqlParser::QuadsContext *ctx) {
		auto triples_template_ctx = ctx->triplesTemplate();
		if (triples_template_ctx) {
			for (auto triples_same_subj_ctx : triples_template_ctx->triplesSameSubject()) {
				visitTriplesSameSubject(triples_same_subj_ctx);
			}
		}
		for (auto quad_detail_ctx : ctx->quadsDetails()) {
			if (quad_detail_ctx->quadsNotTriples()) {
				throw UNSUPPORTED_OP_EXCEPTION(ctx, "GRAPH keyword in DELETE WHERE");
			}
			if (quad_detail_ctx->triplesTemplate()) {
				for (auto triples_same_subj_ctx : triples_template_ctx->triplesSameSubject()) {
					visitTriplesSameSubject(triples_same_subj_ctx);
				}
			}
		}
		return nullptr;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_triples_block(sparql_parser::base::SparqlParser::TriplesBlockContext *ctx) {
		auto parent_scope = current_scope_;
		current_scope_ = {};
		auto operand_dependency_graph = (visit_triples_same_subject_path(ctx->triplesSameSubjectPath(0)));
		for (size_t i = 1; i < ctx->triplesSameSubjectPath().size(); i++) {
			auto temp_odg = (visit_triples_same_subject_path(ctx->triplesSameSubjectPath(i)));
			operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_odg);
		}
		// parent scope should now contain the vars of the graph pattern that was just visited
		parent_scope.insert(current_scope_.begin(), current_scope_.end());
		current_scope_ = parent_scope;
		return operand_dependency_graph;
	}

	std::any QueryUpdateVisitor::visitTriplesSameSubject(sparql_parser::base::SparqlParser::TriplesSameSubjectContext *ctx) {
		if (auto var_or_term_ctx = ctx->varOrTerm(); var_or_term_ctx) {
			active_subject_ = std::any_cast<rdf4cpp::rdf::Node>(visitVarOrTerm(var_or_term_ctx));
			if (auto subj_var = active_subject_.as_variable(); not subj_var.null()) {
				if (in_update_data_)
					throw MALFORMED_QUERY_EXCEPTION(ctx, "Variable node in DELETE/INSERT DATA operation");
				if (subj_var.is_anonymous() and in_delete_template_)
					throw MALFORMED_QUERY_EXCEPTION(ctx, "Blank node in DELETE operation");
				parsed_query_ptr_->track_variable(subj_var);
				parsed_query_ptr_->track_quad_template_variable(subj_var);
			}
			return visitPropertyListNotEmpty(ctx->propertyListNotEmpty());
		}
		throw UNSUPPORTED_OP_EXCEPTION(ctx, "TriplesNodePath");
	}

	std::any QueryUpdateVisitor::visitPropertyListNotEmpty(sparql_parser::base::SparqlParser::PropertyListNotEmptyContext *ctx) {
		for (size_t i = 0; i < ctx->objectList().size(); i++) {
			if (ctx->verb(i)->A()) {
				active_predicate_ = try_get_in_node_storage_else_default(rdf4cpp::rdf::IRI{"http://www.w3.org/1999/02/22-rdf-syntax-ns#type"});
			}
			else if (auto var_ctx = ctx->verb(i)->varOrIRI()->var(); var_ctx) {
				active_predicate_ = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(var_ctx));
				auto pred_var = active_predicate_.as_variable();
				if (in_update_data_)
					throw MALFORMED_QUERY_EXCEPTION(ctx, "Variable node in DELETE/INSERT DATA operation");
				if (pred_var.is_anonymous() and in_delete_template_)
					throw MALFORMED_QUERY_EXCEPTION(ctx, "Blank node in DELETE operation");
				parsed_query_ptr_->track_variable(pred_var);
				parsed_query_ptr_->track_quad_template_variable(pred_var);
			}
			else {
				active_predicate_ = std::any_cast<rdf4cpp::rdf::IRI>(visitIri(ctx->verb(i)->varOrIRI()->iri()));
			}
			for (auto object_ctx : ctx->objectList(i)->object()) {
				if (object_ctx->graphNode()->triplesNode()) {
					throw UNSUPPORTED_OP_EXCEPTION(ctx, "TriplesNode");
				} else {
					active_object_ = std::any_cast<rdf4cpp::rdf::Node>(visitVarOrTerm(object_ctx->graphNode()->varOrTerm()));
					if (auto obj_var = active_object_.as_variable(); not obj_var.null()) {
						if (in_update_data_)
							throw MALFORMED_QUERY_EXCEPTION(ctx, "Variable node in DELETE/INSERT DATA operation");
						if (obj_var.is_anonymous() and in_delete_template_)
							throw MALFORMED_QUERY_EXCEPTION(ctx, "Blank node syntax in DELETE operation");
						parsed_query_ptr_->track_variable(obj_var);
						parsed_query_ptr_->track_quad_template_variable(obj_var);
					}
				}
				quad_patterns_.emplace_back(active_graph_, active_subject_, active_predicate_, active_object_);
			}
		}
		return nullptr;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_triples_same_subject_path(sparql_parser::base::SparqlParser::TriplesSameSubjectPathContext *ctx) {
		if (auto var_or_term_ctx = ctx->varOrTerm(); var_or_term_ctx) {
			active_subject_ = std::any_cast<rdf4cpp::rdf::Node>(visitVarOrTerm(var_or_term_ctx));
			return visit_property_list_path_not_empty(ctx->propertyListPathNotEmpty());
		}
		throw UNSUPPORTED_OP_EXCEPTION(ctx, "TriplesNodePath");
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_property_list_path_not_empty(sparql_parser::base::SparqlParser::PropertyListPathNotEmptyContext *ctx) {
		detail::OperandDependencyGraph operand_dependency_graph{};
		// visit each object list path
		for (auto object_path_ctx : ctx->objectListPath()->objectPath()) {
			detail::OperandDependencyGraph temp_graph{};
			auto graph_node_ctx = object_path_ctx->graphNodePath();
			if (auto var_or_term_ctx = graph_node_ctx->varOrTerm(); var_or_term_ctx) {
				active_object_ = std::any_cast<rdf4cpp::rdf::Node>(visitVarOrTerm(var_or_term_ctx));
			} else {
				if (auto collection_path_ctx = graph_node_ctx->triplesNodePath()->collectionPath(); collection_path_ctx) {
					throw UNSUPPORTED_OP_EXCEPTION(ctx, "CollectionPath");
				}
				temp_graph = (visit_blank_node_property_list_path(graph_node_ctx->triplesNodePath()->blankNodePropertyListPath()));
				operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
			}
			if (auto verb_simple_ctx = ctx->verbSimple(); verb_simple_ctx)
				temp_graph = (visit_verb_simple(verb_simple_ctx));
			else
				temp_graph = (visit_path(ctx->verbPath()->path()));
			operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
		}
		for (auto property_list_ctx : ctx->propertyListPathNotEmptyList()) {
			detail::OperandDependencyGraph temp_graph{};
			for (auto object_ctx : property_list_ctx->objectList()->object()) {
				if (object_ctx->graphNode()->triplesNode()) {
					throw UNSUPPORTED_OP_EXCEPTION(ctx, "TriplesNode");
				} else {
					active_object_ = std::any_cast<rdf4cpp::rdf::Node>(visitVarOrTerm(object_ctx->graphNode()->varOrTerm()));
				}
				if (auto verb_simple_ctx = property_list_ctx->verbSimple(); verb_simple_ctx)
					temp_graph = (visit_verb_simple(verb_simple_ctx));
				else
					temp_graph = (visit_path(property_list_ctx->verbPath()->path()));
				operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
			}
		}
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_blank_node_property_list_path(sparql_parser::base::SparqlParser::BlankNodePropertyListPathContext *ctx) {
		auto current_subject = active_subject_;
		auto b_node_var = variable_from_blank_node("bn_" + std::to_string(bnode_anon_counter_++));
		active_subject_ = b_node_var;
		auto operand_dependency_graph = (visit_property_list_path_not_empty(ctx->propertyListPathNotEmpty()));
		active_subject_ = current_subject;
		active_object_ = b_node_var;
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_verb_simple(sparql_parser::base::SparqlParser::VerbSimpleContext *ctx) {
		active_predicate_ = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(ctx->var()));
		detail::OperandDependencyGraph operand_dependency_graph{};
		create_operand_from_tp(operand_dependency_graph);
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_path(sparql_parser::base::SparqlParser::PathContext *ctx) {
		return visit_path_alternative(ctx->pathAlternative());
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_path_alternative(sparql_parser::base::SparqlParser::PathAlternativeContext *ctx) {
		detail::OperandDependencyGraph operand_dependency_graph{};
		for (auto path_seq_ctx : ctx->pathSequence()) {
			auto temp_graph = (visit_path_sequence(path_seq_ctx));
			operand_dependency_graph = detail::OperandDependencyGraph::merge_graphs(operand_dependency_graph, temp_graph);
		}
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_path_sequence(sparql_parser::base::SparqlParser::PathSequenceContext *ctx) {
		auto path_size = ctx->pathEltOrInverse().size();
		if (path_size == 1) {
			return visit_path_elt_or_inverse(ctx->pathEltOrInverse(0));
		}
		detail::OperandDependencyGraph operand_dependency_graph{};
		// keep track of active_subject_ and active_object_
		auto current_subject = active_subject_;
		auto current_object = active_object_;
		// visit the first
		active_object_ = variable_from_blank_node("bn_" + std::to_string(bnode_anon_counter_++));
		auto temp_graph = (visit_path_elt_or_inverse(ctx->pathEltOrInverse(0)));
		active_subject_ = active_object_;
		operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
		// visit all in the middle
		for (size_t i = 1; i < path_size - 1; i++) {
			active_object_ = variable_from_blank_node("bn_" + std::to_string(bnode_anon_counter_++));
			temp_graph = (visit_path_elt_or_inverse(ctx->pathEltOrInverse(i)));
			operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
			active_subject_ = active_object_;
		}
		// visit the last
		// reset active_object_
		active_object_ = current_object;
		temp_graph = (visit_path_elt_or_inverse(ctx->pathEltOrInverse(path_size - 1)));
		operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
		// reset active_object_
		active_subject_ = current_subject;
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_path_elt_or_inverse(sparql_parser::base::SparqlParser::PathEltOrInverseContext *ctx) {
		bool inverse = ctx->INVERSE() != nullptr;
		if (inverse)
			std::swap(active_subject_, active_object_);
		auto temp_graph = (visit_path_elt(ctx->pathElt()));
		if (inverse)
			std::swap(active_subject_, active_object_);
		return temp_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_path_elt(sparql_parser::base::SparqlParser::PathEltContext *ctx) {
		if (ctx->pathMod())
			throw UNSUPPORTED_OP_EXCEPTION(ctx, "PathMod (*,+,?)");
		auto path_primary_ctx = ctx->pathPrimary();
		if (path_primary_ctx->pathNegatedPropertySet())
			throw UNSUPPORTED_OP_EXCEPTION(ctx, "PathNegatedPropertySet");
		if (auto path_ctx = path_primary_ctx->path(); path_ctx)
			return visit_path(path_ctx);
		active_predicate_ = path_primary_ctx->iri() ? std::any_cast<rdf4cpp::rdf::IRI>(visitIri(path_primary_ctx->iri())) : try_get_in_node_storage_else_default(rdf4cpp::rdf::IRI{"http://www.w3.org/1999/02/22-rdf-syntax-ns#type"});
		detail::OperandDependencyGraph operand_dependency_graph{};
		create_operand_from_tp(operand_dependency_graph);
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_group_or_union_graph_pattern(sparql_parser::base::SparqlParser::GroupOrUnionGraphPatternContext *ctx) {
		detail::OperandDependencyGraph operand_dependency_graph{};
		for (size_t i = 0; i < ctx->groupGraphPattern().size(); i++) {
			auto parent_scope = current_scope_;
			current_scope_ = {};
			auto temp_odg = (visit_group_graph_pattern(ctx->groupGraphPattern(i)));
			operand_dependency_graph = detail::OperandDependencyGraph::merge_graphs(operand_dependency_graph, temp_odg);
			// parent scope should now contain the vars of the graph pattern that was just visited
			parent_scope.insert(current_scope_.begin(), current_scope_.end());
			current_scope_ = parent_scope;
		}
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_optional_graph_pattern(sparql_parser::base::SparqlParser::OptionalGraphPatternContext *ctx) {
		//		auto temp_odg = (visit_group_graph_pattern(ctx->groupGraphPattern(i)));
		//		// parent scope should now contain the vars of the graph pattern that was just visited
		//		parent_scope.insert(current_scope_.begin(), current_scope_.end());
		//		current_scope_ = parent_scope;
		//		return temp_odg;
		// we do not have to change the parent scope for optional
		return visit_group_graph_pattern(ctx->groupGraphPattern());
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_filter(sparql_parser::base::SparqlParser::FilterContext *ctx) {
		// denotes that the visitor is within a filter expression. used for checking whether filter variables are in scope
		in_filter_ = true;
		SPARQLExpressionWrapper expression = [&]() {
			if (auto expr_ctx = ctx->constraint()->expression(); expr_ctx)
				return std::any_cast<SPARQLExpressionWrapper>(visitExpression(expr_ctx));
			else if (auto built_in_call_ctx = ctx->constraint()->builtInCall(); built_in_call_ctx)
				return std::any_cast<SPARQLExpressionWrapper>(visitBuiltInCall(built_in_call_ctx));
			else
				return std::any_cast<SPARQLExpressionWrapper>(visitFunctionCall(ctx->constraint()->functionCall()));
		}();
		detail::OperandDependencyGraph operand_dependency_graph{};
		if (auto and_expr = dynamic_cast<LogicalAndExpression *>(expression.get()); and_expr == nullptr) {
			create_operand_from_filter(operand_dependency_graph, std::move(expression));
		} else {
			std::vector<detail::operand_desc> descriptors{};
			for (auto &&expr : and_expr->expressions()) {
				auto descriptor = create_operand_from_filter(operand_dependency_graph, std::move(expr));
				if (descriptor.has_value())
					descriptors.push_back(descriptor.value());
			}
			create_dependencies_between_operands(operand_dependency_graph, descriptors);
		}
		in_filter_ = false;
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_bind(sparql_parser::base::SparqlParser::BindContext *ctx) {
		in_bind_ = true;
		std::unique_ptr<SPARQLExpression> expr = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression()));
		in_bind_ = false;
		auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(ctx->var()));
		parsed_query_ptr_->register_variable(var);
		parsed_query_ptr_->track_variable(var);
		vars_in_scope_.insert(var);
		if (current_scope_.contains(var))
			throw MALFORMED_QUERY_EXCEPTION(ctx, "BIND variable is already used in the group graph pattern");
		current_scope_.insert(var);
		auto var_id = parsed_query_ptr_->variable_id(var);
		detail::OperandDependencyGraph operand_dependency_graph{};
		std::vector<SPARQLQuery::VarID> vars_ids{};
		for (auto expr_var : expr->variables()) {
			vars_ids.push_back(parsed_query_ptr_->variable_id(expr_var));
		}
		vars_ids.push_back(var_id);
		operand_dependency_graph.add_assignment(vars_ids, detail::AssignmentExpression(std::move(expr), parsed_query_ptr_->variable_id(var)));
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_inline_data(sparql_parser::base::SparqlParser::InlineDataContext *ctx) {
		if (auto one_var_ctx = ctx->dataBlock()->inlineDataOneVar(); one_var_ctx)
			return visit_inline_data_one_var(one_var_ctx);
		return visit_inline_data_full(ctx->dataBlock()->inlineDataFull());
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_inline_data_one_var(sparql_parser::base::SparqlParser::InlineDataOneVarContext *ctx) {
		auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(ctx->var()));
		parsed_query_ptr_->register_variable(var);
		vars_in_scope_.insert(var);
		current_scope_.insert(var);
		detail::BoolHypertrie data{1, parsed_query_ptr_->tensor_context().get()};
		for (auto data_block_value_ctx : ctx->dataBlockValue()) {
			if (auto iri_ctx = data_block_value_ctx->iri(); iri_ctx)
				data.set({std::any_cast<rdf4cpp::rdf::IRI>(visitIri(iri_ctx))}, true);
			else if (auto literal_ctx = data_block_value_ctx->rdfLiteral(); literal_ctx)
				data.set({std::any_cast<rdf4cpp::rdf::Literal>(visitRdfLiteral(literal_ctx))}, true);
			else if (auto num_literal_ctx = data_block_value_ctx->numericLiteral(); num_literal_ctx)
				data.set({std::any_cast<rdf4cpp::rdf::Literal>(visitNumericLiteral(num_literal_ctx))}, true);
			else if (auto bool_literal = data_block_value_ctx->booleanLiteral(); bool_literal)
				data.set({std::any_cast<rdf4cpp::rdf::Literal>(visitBooleanLiteral(bool_literal))}, true);
			else
				throw UNSUPPORTED_OP_EXCEPTION(ctx, "UNDEF in InlineData");
		}
		auto id = parsed_query_ptr_->add_inline_data(std::move(data));
		detail::OperandDependencyGraph operand_dependency_graph{};
		operand_dependency_graph.add_inline_data({parsed_query_ptr_->variable_id(var)}, id);
		return operand_dependency_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::visit_inline_data_full(sparql_parser::base::SparqlParser::InlineDataFullContext *ctx) {
		std::vector<SPARQLQuery::VarID> var_ids{};
		for (auto var_ctx : ctx->var()) {
			auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(var_ctx));
			parsed_query_ptr_->register_variable(var);
			vars_in_scope_.insert(var);
			current_scope_.insert(var);
			var_ids.push_back(parsed_query_ptr_->variable_id(var));
		}
		if (var_ids.size() > 3)
			throw UNSUPPORTED_OP_EXCEPTION(ctx, "InlineData supports up to three variables");
		detail::BoolHypertrie data{var_ids.size(), parsed_query_ptr_->tensor_context().get()};
		for (auto data_block_value_ctx : ctx->dataBlockValues()) {
			auto key = hypertrie::Key<detail::htt_t>::make_defaulted(var_ids.size());
			for (size_t i = 0; i < var_ids.size(); i++) {
				if (auto iri_ctx = data_block_value_ctx->dataBlockValue(i)->iri(); iri_ctx)
					key[i] = std::any_cast<rdf4cpp::rdf::IRI>(visitIri(iri_ctx));
				else if (auto literal_ctx = data_block_value_ctx->dataBlockValue(i)->rdfLiteral(); literal_ctx)
					key[i] = std::any_cast<rdf4cpp::rdf::Literal>((visitRdfLiteral(literal_ctx)));
				else if (auto num_literal_ctx = data_block_value_ctx->dataBlockValue(i)->numericLiteral(); num_literal_ctx)
					key[i] = std::any_cast<rdf4cpp::rdf::Literal>((visitNumericLiteral(num_literal_ctx)));
				else if (auto bool_literal = data_block_value_ctx->dataBlockValue(i)->booleanLiteral(); bool_literal)
					key[i] = std::any_cast<rdf4cpp::rdf::Literal>(visitBooleanLiteral(bool_literal));
				else
					throw UNSUPPORTED_OP_EXCEPTION(ctx, "UNDEF in InlineData");
			}
			data.set(key, true);
		}
		auto id = parsed_query_ptr_->add_inline_data(std::move(data));
		detail::OperandDependencyGraph operand_dependency_graph{};
		operand_dependency_graph.add_inline_data(var_ids, id);
		return operand_dependency_graph;
	}

	std::any QueryUpdateVisitor::visitGroupClause(sparql_parser::base::SparqlParser::GroupClauseContext *ctx) {
		has_grouping_ = true;
		for (auto group_condition : ctx->groupCondition()) {
			if (group_condition->builtInCall()) {
				auto expr = std::any_cast<SPARQLExpressionWrapper>(visitBuiltInCall(group_condition->builtInCall()));
				parsed_query_ptr_->add_grouping_expression(std::move(expr));
			} else if (group_condition->functionCall()) {
				auto expr = std::any_cast<SPARQLExpressionWrapper>(visitFunctionCall(group_condition->functionCall()));
				parsed_query_ptr_->add_grouping_expression(std::move(expr));
			} else if (group_condition->var() and not group_condition->expression()) {
				auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(group_condition->var()));
				parsed_query_ptr_->add_grouping_expression(check_for_alias(var));
				vars_in_group_by_.insert(var);
			} else if (group_condition->expression()) {
				auto expr = std::any_cast<SPARQLExpressionWrapper>(visitExpression(group_condition->expression()));
				if (group_condition->AS()) {
					auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(group_condition->var()));
					if (vars_in_scope_.contains(var))
						throw MALFORMED_QUERY_EXCEPTION(ctx, "GROUP BY Variable " + var.backend_handle().variable_backend().n_string() + " is already in scope.");
					vars_in_scope_.insert(var);
					vars_in_group_by_.insert(var);
					aliases_.emplace(var, expr);
				}
				parsed_query_ptr_->add_grouping_expression(std::move(expr));
			} else {
				assert(false);
			}
		}
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitHavingClause(sparql_parser::base::SparqlParser::HavingClauseContext *ctx) {
		for (auto cond_ctx : ctx->havingCondition()) {
			std::unique_ptr<SPARQLExpression> expr = nullptr;
			rdf4cpp::rdf::query::Variable var{};
			if (auto expr_ctx = cond_ctx->constraint()->expression(); expr_ctx) {
				expr = std::any_cast<SPARQLExpressionWrapper>(visitExpression(expr_ctx));
			} else if (auto built_in_ctx = cond_ctx->constraint()->builtInCall(); built_in_ctx) {
				expr = std::any_cast<SPARQLExpressionWrapper>(visitBuiltInCall(built_in_ctx));
			} else {
				expr = std::any_cast<SPARQLExpressionWrapper>(visitFunctionCall(cond_ctx->constraint()->functionCall()));
			}
			for (auto const &non_aggr_var : expr->non_aggregated_variables()) {
				non_agg_vars_in_having_.insert(non_aggr_var);
			}
			parsed_query_ptr_->add_having_expression(SPARQLExpressionWrapper{std::move(expr)});
		}
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitOrderClause(sparql_parser::base::SparqlParser::OrderClauseContext *ctx) {
		for (auto cond_ctx : ctx->orderCondition()) {
			std::unique_ptr<SPARQLExpression> expr = nullptr;
			bool desc = false;
			if (cond_ctx->DESC())
				desc = true;
			if (cond_ctx->expression()) {
				expr = std::any_cast<SPARQLExpressionWrapper>(visitExpression(cond_ctx->expression()));
				for (auto const &var : expr->non_aggregated_variables()) {
					non_agg_vars_in_order_by_.insert(var);
				}
			} else if (auto constraint_ctx = cond_ctx->constraint(); constraint_ctx) {
				if (constraint_ctx->expression())
					expr = std::any_cast<SPARQLExpressionWrapper>(visitExpression(constraint_ctx->expression()));
				else if (constraint_ctx->builtInCall())
					expr = std::any_cast<SPARQLExpressionWrapper>(visitBuiltInCall(constraint_ctx->builtInCall()));
				else
					expr = std::any_cast<SPARQLExpressionWrapper>(visitFunctionCall(constraint_ctx->functionCall()));
				for (auto const &var : expr->non_aggregated_variables()) {
					non_agg_vars_in_order_by_.insert(var);
				}
			} else if (cond_ctx->var()) {
				auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(cond_ctx->var()));
				expr = check_for_alias(var);
				non_agg_vars_in_order_by_.insert(var);
			} else {
				assert(false);
			}
			parsed_query_ptr_->add_ordering(SPARQLExpressionWrapper{std::move(expr)}, desc);
		}
		return nullptr;
	}

	std::any QueryUpdateVisitor::visitLimitOffsetClauses(sparql_parser::base::SparqlParser::LimitOffsetClausesContext *ctx) {
		if (auto limit_ctx = ctx->limitClause(); limit_ctx)
			parsed_query_ptr_->set_limit(std::stoi(limit_ctx->INTEGER()->getText()));
		if (auto offset_ctx = ctx->offsetClause(); offset_ctx)
			parsed_query_ptr_->set_offset(std::stoi(offset_ctx->INTEGER()->getText()));
		return nullptr;
	}

	/* visit functions for expressions */

	std::any QueryUpdateVisitor::visitExpression(sparql_parser::base::SparqlParser::ExpressionContext *ctx) {
		if (auto base_ctx = dynamic_cast<sparql_parser::base::SparqlParser::BaseExpressionContext *>(ctx); base_ctx)
			return visitPrimaryExpression(base_ctx->primaryExpression());
		if (auto and_ctx = dynamic_cast<sparql_parser::base::SparqlParser::ConditionalAndExpressionContext *>(ctx); and_ctx)
			return visitConditionalAndExpression(and_ctx);
		if (auto or_ctx = dynamic_cast<sparql_parser::base::SparqlParser::ConditionalOrExpressionContext *>(ctx); or_ctx)
			return visitConditionalOrExpression(or_ctx);
		if (auto relational_ctx = dynamic_cast<sparql_parser::base::SparqlParser::RelationalExpressionContext *>(ctx); relational_ctx)
			return visitRelationalExpression(relational_ctx);
		if (auto relational_set_ctx = dynamic_cast<sparql_parser::base::SparqlParser::RelationalSetExpressionContext *>(ctx); relational_set_ctx)
			return visitRelationalSetExpression(relational_set_ctx);
		if (auto unary_neg_ctx = dynamic_cast<sparql_parser::base::SparqlParser::UnaryNegationExpressionContext *>(ctx); unary_neg_ctx)
			return visitUnaryNegationExpression(unary_neg_ctx);
		if (auto additive_ctx = dynamic_cast<sparql_parser::base::SparqlParser::AdditiveExpressionContext *>(ctx); additive_ctx)
			return visitAdditiveExpression(additive_ctx);
		if (auto multiplicative_ctx = dynamic_cast<sparql_parser::base::SparqlParser::MultiplicativeExpressionContext *>(ctx); multiplicative_ctx)
			return visitMultiplicativeExpression(multiplicative_ctx);
		if (auto unary_additive_ctx = dynamic_cast<sparql_parser::base::SparqlParser::UnaryAdditiveExpressionContext *>(ctx); unary_additive_ctx)
			return visitUnaryAdditiveExpression(unary_additive_ctx);
		if (auto unary_signed_literal_ctx = dynamic_cast<sparql_parser::base::SparqlParser::UnarySignedLiteralExpressionContext *>(ctx); unary_signed_literal_ctx)
			return visitUnarySignedLiteralExpression(unary_signed_literal_ctx);
		throw UNSUPPORTED_OP_EXCEPTION(ctx, ctx->getText());
	}

	std::any QueryUpdateVisitor::visitPrimaryExpression(sparql_parser::base::SparqlParser::PrimaryExpressionContext *ctx) {
		if (ctx->var()) {
			auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(ctx->var()));
			// in case of filter, check if var is in scope of the group graph pattern. if it is not, create an anonymous variable
			// the anonymous variable will be registered and tracked; however, it will remain unbound
			if ((in_filter_ or in_bind_) and not current_scope_.contains(var)) {
				var = variable_from_blank_node("bn_" + std::to_string(bnode_anon_counter_++));
			}
			return check_for_alias(var);
		}
		if (ctx->rdfLiteral()) {
			auto rdf_literal = std::any_cast<rdf4cpp::rdf::Literal>(visitRdfLiteral(ctx->rdfLiteral()));
			return SPARQLExpressionWrapper{std::make_unique<PrimaryLiteralExpression>(rdf_literal)};
		}
		if (ctx->booleanLiteral()) {
			auto boolean_literal = std::any_cast<rdf4cpp::rdf::Literal>(visitBooleanLiteral(ctx->booleanLiteral()));
			return SPARQLExpressionWrapper{std::make_unique<PrimaryLiteralExpression>(boolean_literal)};
		}
		if (ctx->numericLiteral()) {
			auto numeric_literal = std::any_cast<rdf4cpp::rdf::Literal>(visitNumericLiteral(ctx->numericLiteral()));
			return SPARQLExpressionWrapper{std::make_unique<PrimaryLiteralExpression>(numeric_literal)};
		}
		if (auto built_in_call_ctx = ctx->builtInCall(); built_in_call_ctx) {
			if (auto aggregate_ctx = built_in_call_ctx->aggregate(); aggregate_ctx)
				return visitAggregate(aggregate_ctx);
			else
				return visitBuiltInCall(built_in_call_ctx);
		}
		if (auto iri_or_function_ctx = ctx->iriRefOrFunction(); iri_or_function_ctx) {
			if (iri_or_function_ctx->argList()) {
				return visitFunctionCall_impl(iri_or_function_ctx);
			} else {
				auto iri = std::any_cast<rdf4cpp::rdf::IRI>(visitIri(iri_or_function_ctx->iri()));
				return SPARQLExpressionWrapper{std::make_unique<PrimaryIRIExpression>(iri)};
			}
		}
		return visitExpression(ctx->expression());
	}

	std::any QueryUpdateVisitor::visitConditionalAndExpression(sparql_parser::base::SparqlParser::ConditionalAndExpressionContext *ctx) {
		std::vector<std::unique_ptr<SPARQLExpression>> expressions;
		for (auto expr_ctx : ctx->expression()) {
			expressions.push_back(std::any_cast<SPARQLExpressionWrapper>(visitExpression(expr_ctx)));
		}
		return SPARQLExpressionWrapper{std::make_unique<LogicalAndExpression>(std::move(expressions))};
	}

	std::any QueryUpdateVisitor::visitConditionalOrExpression(sparql_parser::base::SparqlParser::ConditionalOrExpressionContext *ctx) {
		std::vector<std::unique_ptr<SPARQLExpression>> expressions;
		for (auto expr_ctx : ctx->expression()) {
			expressions.push_back(std::any_cast<SPARQLExpressionWrapper>(visitExpression(expr_ctx)));
		}
		return SPARQLExpressionWrapper{std::make_unique<LogicalOrExpression>(std::move(expressions))};
	}

	std::any QueryUpdateVisitor::visitAdditiveExpression(sparql_parser::base::SparqlParser::AdditiveExpressionContext *ctx) {
		auto lhs = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
		auto rhs = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
		if (ctx->PLUS_SIGN())
			return SPARQLExpressionWrapper{std::make_unique<AdditionExpression>(std::move(lhs), std::move(rhs))};
		return SPARQLExpressionWrapper{std::make_unique<SubtractionExpression>(std::move(lhs), std::move(rhs))};
	}

	std::any QueryUpdateVisitor::visitMultiplicativeExpression(sparql_parser::base::SparqlParser::MultiplicativeExpressionContext *ctx) {
		auto lhs = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
		auto rhs = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
		if (ctx->ASTERISK())
			return SPARQLExpressionWrapper{std::make_unique<MultiplicationExpression>(std::move(lhs), std::move(rhs))};
		return SPARQLExpressionWrapper{std::make_unique<DivisionExpression>(std::move(lhs), std::move(rhs))};
	}

	std::any QueryUpdateVisitor::visitUnaryAdditiveExpression(sparql_parser::base::SparqlParser::UnaryAdditiveExpressionContext *ctx) {
		if (ctx->PLUS_SIGN())
			return SPARQLExpressionWrapper{std::make_unique<UnaryPlusExpression>(std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression())))};
		return SPARQLExpressionWrapper{std::make_unique<UnaryMinusExpression>(std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression())))};
	}

	std::any QueryUpdateVisitor::visitUnarySignedLiteralExpression(sparql_parser::base::SparqlParser::UnarySignedLiteralExpressionContext *ctx) {
		auto expr = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression()));
		auto unary_expr = std::any_cast<SPARQLExpressionWrapper>(visitUnaryLiteralExpression(ctx->unaryLiteralExpression()));
		return SPARQLExpressionWrapper{std::make_unique<AdditionExpression>(std::move(expr), std::move(unary_expr))};
	}

	std::any QueryUpdateVisitor::visitUnaryLiteralExpression(sparql_parser::base::SparqlParser::UnaryLiteralExpressionContext *ctx) {
		std::unique_ptr<SPARQLExpression> literal_expr;
		if (auto positive_literal_ctx = ctx->numericLiteralPositive(); positive_literal_ctx)
			literal_expr = std::make_unique<PrimaryLiteralExpression>(std::any_cast<rdf4cpp::rdf::Literal>(visitNumericLiteralPositive(positive_literal_ctx)));
		else
			literal_expr = std::make_unique<PrimaryLiteralExpression>(std::any_cast<rdf4cpp::rdf::Literal>(visitNumericLiteralNegative(ctx->numericLiteralNegative())));
		return SPARQLExpressionWrapper{std::move(literal_expr)};
	}

	std::any QueryUpdateVisitor::visitRelationalExpression(sparql_parser::base::SparqlParser::RelationalExpressionContext *ctx) {
		std::unique_ptr<SPARQLExpression> lhs_op = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
		std::unique_ptr<SPARQLExpression> rhs_op = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
		return SPARQLExpressionWrapper{[&]() -> std::unique_ptr<SPARQLExpression> {
			if (ctx->EQUAL())
				return std::make_unique<EqualsExpression>(std::move(lhs_op), std::move(rhs_op));
			if (ctx->NOT_EQUAL())
				return std::make_unique<NotEqualsExpression>(std::move(lhs_op), std::move(rhs_op));
			if (ctx->GREATER())
				return std::make_unique<GreaterExpression>(std::move(lhs_op), std::move(rhs_op));
			if (ctx->GREATER_EQUAL())
				return std::make_unique<GreaterEqualsExpression>(std::move(lhs_op), std::move(rhs_op));
			if (ctx->LESS())
				return std::make_unique<LessExpression>(std::move(lhs_op), std::move(rhs_op));
			return std::make_unique<LessEqualsExpression>(std::move(lhs_op), std::move(rhs_op));
		}()};
	}

	std::any QueryUpdateVisitor::visitRelationalSetExpression(sparql_parser::base::SparqlParser::RelationalSetExpressionContext *ctx) {
		std::unique_ptr<SPARQLExpression> lhs_op = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression()));
		std::vector<std::unique_ptr<SPARQLExpression>> expression_list{};
		if (ctx->expressionList()) {
			for (auto &expr_ctx : ctx->expressionList()->expression()) {
				expression_list.push_back(std::any_cast<SPARQLExpressionWrapper>(visitExpression(expr_ctx)));
			}
		}
		bool not_in = ctx->NOT() != nullptr;
		return SPARQLExpressionWrapper{std::make_unique<InExpressionList>(std::move(lhs_op), std::move(expression_list), not_in)};
	}

	std::any QueryUpdateVisitor::visitUnaryNegationExpression(sparql_parser::base::SparqlParser::UnaryNegationExpressionContext *ctx) {
		return SPARQLExpressionWrapper{std::make_unique<NotExpression>(std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression())))};
	}

	std::any QueryUpdateVisitor::visitBuiltInCall(sparql_parser::base::SparqlParser::BuiltInCallContext *ctx) {
		// treat EXISTS differently from the other built-in calls
		if (auto exists_ctx = ctx->existsFunction(); exists_ctx) {
			return visitExists(exists_ctx->groupGraphPattern(), false);
		}
		if (auto not_exists_ctx = ctx->notExistsFunction(); not_exists_ctx) {
			return visitExists(not_exists_ctx->groupGraphPattern(), true);
		}
		// treat BOUND differently from the other built-in calls
		if (ctx->BOUND()) {
			auto var = std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(ctx->var()));
			return SPARQLExpressionWrapper{std::make_unique<Bound>(check_for_alias(var))};
		}
		// built-in function calls
		return SPARQLExpressionWrapper{[&]() -> std::unique_ptr<SPARQLExpression> {
			/* no arguments */
			if (ctx->UUID()) {
				return std::make_unique<UUID>();
			} else if (ctx->STRUUID()) {
				return std::make_unique<StrUUID>();
			} else if (ctx->RAND()) {
				return std::make_unique<Rand>();
			} else if (ctx->NOW()) {
				return std::make_unique<Now>();
			}
			/* single argument */
			else if (ctx->ISIRI() or ctx->ISURI()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<IsIRI>(std::move(first));
			} else if (ctx->IRI() or ctx->URI()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<IRI>(std::move(first));
			}else if (ctx->ISBLANK()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<IsBlank>(std::move(first));
			} else if (ctx->ISLITERAL()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<IsLiteral>(std::move(first));
			} else if (ctx->DATATYPE()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Datatype>(std::move(first));
			} else if (ctx->STR()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Str>(std::move(first));
			} else if (ctx->LANG()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Lang>(std::move(first));
			} else if (ctx->STRLEN()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<StrLen>(std::move(first));
			} else if (ctx->UCASE()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<UCase>(std::move(first));
			} else if (ctx->LCASE()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<LCase>(std::move(first));
			} else if (ctx->ENCODE_FOR_URI()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<EncodeForURI>(std::move(first));
			} else if (ctx->ISNUMERIC()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<IsNumeric>(std::move(first));
			} else if (ctx->ABS()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Abs>(std::move(first));
			} else if (ctx->ROUND()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Round>(std::move(first));
			} else if (ctx->CEIL()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Ceil>(std::move(first));
			} else if (ctx->FLOOR()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Floor>(std::move(first));
			} else if (ctx->MD5()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<MD5>(std::move(first));
			} else if (ctx->SHA1()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<SHA1>(std::move(first));
			} else if (ctx->SHA256()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<SHA256>(std::move(first));
			} else if (ctx->SHA384()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<SHA384>(std::move(first));
			} else if (ctx->SHA512()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<SHA512>(std::move(first));
			} else if (ctx->YEAR()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Year>(std::move(first));
			} else if (ctx->MONTH()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Month>(std::move(first));
			} else if (ctx->DAY()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Day>(std::move(first));
			} else if (ctx->HOURS()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Hours>(std::move(first));
			} else if (ctx->MINUTES()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Minutes>(std::move(first));
			} else if (ctx->SECONDS()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Seconds>(std::move(first));
			} else if (ctx->TIMEZONE()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<Timezone>(std::move(first));
			} else if (ctx->TZ()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				return std::make_unique<TZ>(std::move(first));
			}
			else if (ctx->BNODE()) {
				throw UNSUPPORTED_OP_EXCEPTION(ctx, ctx->getText());
			}
			/* two arguments */
			else if (ctx->SAMETERM()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<SameTerm>(std::move(first), std::move(second));
			}
			else if (ctx->CONTAINS()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<Contains>(std::move(first), std::move(second));
			} else if (ctx->STRSTARTS()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<StrStarts>(std::move(first), std::move(second));
			} else if (ctx->STRBEFORE()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<StrBefore>(std::move(first), std::move(second));
			} else if (ctx->STRAFTER()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<StrAfter>(std::move(first), std::move(second));
			} else if (ctx->STRENDS()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<StrEnds>(std::move(first), std::move(second));
			} else if (ctx->STRLANG()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<StrLang>(std::move(first), std::move(second));
			} else if (ctx->LANGMATCHES()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<LangMatches>(std::move(first), std::move(second));
			} else if (ctx->STRDT()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				return std::make_unique<StrDt>(std::move(first), std::move(second));
			}
			/* three arguments */
			else if (ctx->subStringExpression()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->subStringExpression()->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->subStringExpression()->expression(1)));
				if (ctx->subStringExpression()->expression().size() > 2) {
					auto third = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->subStringExpression()->expression(2)));
					return std::make_unique<SubStr3>(std::move(first), std::move(second), std::move(third));
				} else {
					return std::make_unique<SubStr2>(std::move(first), std::move(second));
				}
			} else if (ctx->IF()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(1)));
				auto third = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression(2)));
				return std::make_unique<If>(std::move(first), std::move(second), std::move(third));
			}
			else if (ctx->regexExpression()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->regexExpression()->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->regexExpression()->expression(1)));
				if (ctx->regexExpression()->expression().size() > 2) {
				auto third = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->regexExpression()->expression(2)));
					return std::make_unique<Regex3>(std::move(first), std::move(second), std::move(third));
				} else {
					return std::make_unique<Regex2>(std::move(first), std::move(second));
				}
			}
			/* four arguments */
			else if (ctx->strReplaceExpression()) {
				auto first = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->strReplaceExpression()->expression(0)));
				auto second = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->strReplaceExpression()->expression(1)));
				auto third = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->strReplaceExpression()->expression(2)));
				if (ctx->strReplaceExpression()->expression().size() > 3) {
					auto fourth = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->strReplaceExpression()->expression(3)));
					return std::make_unique<Replace4>(std::move(first), std::move(second), std::move(third), std::move(fourth));
				} else {
					return std::make_unique<Replace3>(std::move(first), std::move(second), std::move(third));
				}
			}
			/* list of arguments */
			else if (ctx->CONCAT()) {
				std::vector<std::unique_ptr<SPARQLExpression>> expr_args{};
				for (auto expr : ctx->expressionList()->expression()) {
					expr_args.push_back(std::any_cast<SPARQLExpressionWrapper>(visitExpression(expr)));
				}
				return std::make_unique<Concat>(std::move(expr_args));
			} else if (ctx->COALESCE()) {
				std::vector<std::unique_ptr<SPARQLExpression>> expr_args{};
				for (auto expr : ctx->expressionList()->expression()) {
					expr_args.push_back(std::any_cast<SPARQLExpressionWrapper>(visitExpression(expr)));
				}
				return std::make_unique<Coalesce>(std::move(expr_args));
			}
			else {
				throw UNSUPPORTED_OP_EXCEPTION(ctx, ctx->getText());
			}
		}()};
	}

	template<typename T>
	std::any QueryUpdateVisitor::visitFunctionCall_impl(T *ctx) {
		auto f_iri = std::any_cast<rdf4cpp::rdf::IRI>(visitIri(ctx->iri()));
		auto f_identifier = f_iri.identifier();
		if (f_identifier.starts_with("http://www.w3.org/2001/XMLSchema#"))
			return visitXSDFunction_impl(f_identifier, ctx);
		else
			throw UNSUPPORTED_OP_EXCEPTION(ctx, "Unsupported function: " + ctx->iri()->getText());
	}

	std::any QueryUpdateVisitor::visitFunctionCall(sparql_parser::base::SparqlParser::FunctionCallContext *ctx) {
		return visitFunctionCall_impl(ctx);
	}

	template<typename T>
	SPARQLExpressionWrapper QueryUpdateVisitor::visitXSDFunction_impl(std::string_view const &identifier, T *ctx) {
		using namespace expressions::functions;
		std::vector<SPARQLExpressionWrapper> expr_args{};
		auto arg_list_ctx = ctx->argList();
		if (arg_list_ctx->DISTINCT())
			throw MALFORMED_QUERY_EXCEPTION(ctx, "DISTINCT keyword is only allowed in aggregate functions.");
		for (auto expr : arg_list_ctx->expressionList()->expression()) {
			expr_args.push_back(std::any_cast<SPARQLExpressionWrapper>(visitExpression(expr)));
		}
		if (identifier == rdf4cpp::rdf::datatypes::registry::xsd_boolean) {
			if (expr_args.size() != 1)
				throw MALFORMED_QUERY_EXCEPTION(ctx, "Argument mismatch: " + ctx->iri()->getText());
			return SPARQLExpressionWrapper(std::make_unique<BooleanConstructor>(std::move(expr_args[0])));
		} else if (identifier == rdf4cpp::rdf::datatypes::registry::xsd_decimal) {
			if (expr_args.size() != 1)
				throw MALFORMED_QUERY_EXCEPTION(ctx, "Argument mismatch: " + ctx->iri()->getText());
			return SPARQLExpressionWrapper(std::make_unique<DecimalConstructor>(std::move(expr_args[0])));
		} else if (identifier == rdf4cpp::rdf::datatypes::registry::xsd_double) {
			if (expr_args.size() != 1)
				throw MALFORMED_QUERY_EXCEPTION(ctx, "Argument mismatch: " + ctx->iri()->getText());
			return SPARQLExpressionWrapper(std::make_unique<DoubleConstructor>(std::move(expr_args[0])));
		} else if (identifier == rdf4cpp::rdf::datatypes::registry::xsd_float) {
			if (expr_args.size() != 1)
				throw MALFORMED_QUERY_EXCEPTION(ctx, "Argument mismatch: " + ctx->iri()->getText());
			return SPARQLExpressionWrapper(std::make_unique<FloatConstructor>(std::move(expr_args[0])));
		} else if (identifier == rdf4cpp::rdf::datatypes::registry::xsd_integer) {
			if (expr_args.size() != 1)
				throw MALFORMED_QUERY_EXCEPTION(ctx, "Argument mismatch: " + ctx->iri()->getText());
			return SPARQLExpressionWrapper(std::make_unique<IntegerConstructor>(std::move(expr_args[0])));
		} else if (identifier == rdf4cpp::rdf::datatypes::registry::xsd_string) {
			if (expr_args.size() != 1)
				throw MALFORMED_QUERY_EXCEPTION(ctx, "Argument mismatch: " + ctx->iri()->getText());
			return SPARQLExpressionWrapper(std::make_unique<StringConstructor>(std::move(expr_args[0])));
		}
		throw UNSUPPORTED_OP_EXCEPTION(ctx, "Unsupported function: " + ctx->iri()->getText());
	}

	SPARQLExpressionWrapper QueryUpdateVisitor::visitXSDFunction(std::string_view const &identifier, sparql_parser::base::SparqlParser::FunctionCallContext *ctx) {
		return visitXSDFunction_impl(identifier, ctx);
	}

	std::any QueryUpdateVisitor::visitAggregate(sparql_parser::base::SparqlParser::AggregateContext *ctx) {
		// todo: how should we treat count(*) in order by and having?
		parsed_query_ptr_->set_aggregates();
		std::unique_ptr<SPARQLExpression> expr;
		std::unique_ptr<SPARQLExpression> nested_expr;
		if (ctx->expression()) {
			nested_expr = std::any_cast<SPARQLExpressionWrapper>(visitExpression(ctx->expression()));
			if (not nested_expr->aggregated_variables().empty()) {
				throw MALFORMED_QUERY_EXCEPTION(ctx, "Nested aggregates are not allowed");
			}
		}
		if (ctx->DISTINCT()) {
			if (ctx->COUNT()) {
				if (ctx->ASTERISK()) {
					for (auto const &var : vars_in_scope_) {
						parsed_query_ptr_->track_variable(var);
					}
					expr = std::make_unique<CountStarDistinct>();
				} else {
					expr = std::make_unique<CountDistinct>(std::move(nested_expr));
				}
			} else if (ctx->MIN()) {
				expr = std::make_unique<Min>(std::move(nested_expr));
			} else if (ctx->MAX()) {
				expr = std::make_unique<Max>(std::move(nested_expr));
			} else if (ctx->SAMPLE()) {
				expr = std::make_unique<Sample>(std::move(nested_expr));
			} else if (ctx->SUM()) {
				expr = std::make_unique<SumDistinct>(std::move(nested_expr));
			} else if (ctx->AVG()) {
				expr = std::make_unique<Avg>(std::move(nested_expr), true);
			} else {
				throw UNSUPPORTED_OP_EXCEPTION(ctx, "Aggregate function not supported: " + ctx->getText());
			}
		} else {
			if (ctx->COUNT()) {
				if (ctx->ASTERISK()) {
					for (auto const &var : vars_in_scope_) {
						parsed_query_ptr_->track_variable(var);
					}
					expr = std::make_unique<CountStar>();
				} else {
					expr = std::make_unique<Count>(std::move(nested_expr));
				}
			} else if (ctx->MIN()) {
				expr = std::make_unique<Min>(std::move(nested_expr));
			} else if (ctx->MAX()) {
				expr = std::make_unique<Max>(std::move(nested_expr));
			} else if (ctx->SAMPLE()) {
				expr = std::make_unique<Sample>(std::move(nested_expr));
			} else if (ctx->SUM()) {
				expr = std::make_unique<Sum>(std::move(nested_expr));
			} else if (ctx->AVG()) {
				expr = std::make_unique<Avg>(std::move(nested_expr));
			} else {
				throw UNSUPPORTED_OP_EXCEPTION(ctx, "Aggregate function not supported: " + ctx->getText());
			}
		}
		return SPARQLExpressionWrapper{std::move(expr)};
	}

	SPARQLExpressionWrapper QueryUpdateVisitor::visitExists(sparql_parser::base::SparqlParser::GroupGraphPatternContext *ctx, bool is_not) {
		// treat the pattern of the EXIST function as a subquery
		SPARQLQuery sub_query{parsed_query_ptr_->tensor_context()};
		QueryUpdateVisitor sub_query_visitor(sub_query, *prefixes_ptr_, node_storage_);
		sub_query.set_operand_dependency_graph((sub_query_visitor.visit_group_graph_pattern(ctx)));
		// associate the variables of the EXIST subquery with the current query
		boost::container::flat_map<char, size_t> subquery_var_ids_positions{};
		std::vector<rdf4cpp::rdf::query::Variable> vars{};
		for (auto const &sub_query_var : sub_query_visitor.vars_in_scope_) {
			if (current_scope_.contains(sub_query_var)) {
				vars.push_back(sub_query_var);
				parsed_query_ptr_->track_variable(sub_query_var);
				subquery_var_ids_positions[sub_query.variable_id(sub_query_var)] = parsed_query_ptr_->tracked_variable_position(sub_query_var);
			}
		}
		return SPARQLExpressionWrapper{
				std::make_unique<Exists>(std::move(vars), std::move(subquery_var_ids_positions), sub_query.raw_query(), is_not)};
	}

	/* visit functions for terms */

	std::any QueryUpdateVisitor::visitVarOrTerm(sparql_parser::base::SparqlParser::VarOrTermContext *ctx) {
		return [&]() -> rdf4cpp::rdf::Node {// the lambda takes care of conversion to rdf::Node
			if (ctx->var()) {
				return std::any_cast<rdf4cpp::rdf::query::Variable>(visitVar(ctx->var()));
			} else {
				if (auto iri_ctx = ctx->graphTerm()->iri())
					return std::any_cast<rdf4cpp::rdf::IRI>(visitIri(iri_ctx));
				else if (auto blank_node_ctx = ctx->graphTerm()->blankNode(); blank_node_ctx)
					return std::any_cast<rdf4cpp::rdf::query::Variable>(visitBlankNode(blank_node_ctx));
				else if (auto rdf_literal_ctx = ctx->graphTerm()->rdfLiteral(); rdf_literal_ctx)
					return std::any_cast<rdf4cpp::rdf::Literal>(visitRdfLiteral(rdf_literal_ctx));
				else if (auto boolean_literal_ctx = ctx->graphTerm()->booleanLiteral(); boolean_literal_ctx)
					return std::any_cast<rdf4cpp::rdf::Literal>(visitBooleanLiteral(boolean_literal_ctx));
				else if (auto numberic_literal_ctx = ctx->graphTerm()->numericLiteral(); numberic_literal_ctx)
					return std::any_cast<rdf4cpp::rdf::Literal>(visitNumericLiteral(numberic_literal_ctx));
				throw UNSUPPORTED_OP_EXCEPTION(ctx, "NIL and RDF collections.");
			}
		}();
	}

	std::any QueryUpdateVisitor::visitVar(sparql_parser::base::SparqlParser::VarContext *ctx) {
		auto var = rdf4cpp::rdf::query::Variable(ctx->getText().substr(1));
		parsed_query_ptr_->register_variable(var);
		return var;
	}

	std::any QueryUpdateVisitor::visitRdfLiteral(sparql_parser::base::SparqlParser::RdfLiteralContext *ctx) {
		auto value = std::any_cast<std::string>(visitString(ctx->string()));
		if (auto iri_ctx = ctx->iri(); iri_ctx)
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed(value, std::any_cast<rdf4cpp::rdf::IRI>(visitIri(iri_ctx))));
		else if (auto lang_tag_ctx = ctx->LANGTAG(); lang_tag_ctx)
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_lang_tagged(value, lang_tag_ctx->getText().substr(1)));
		else
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_simple(value));
	}

	std::any QueryUpdateVisitor::visitNumericLiteral(sparql_parser::base::SparqlParser::NumericLiteralContext *ctx) {
		if (auto pos_literal_ctx = ctx->numericLiteralPositive(); pos_literal_ctx)
			return visitNumericLiteralPositive(pos_literal_ctx);
		if (auto neg_literal_ctx = ctx->numericLiteralNegative(); neg_literal_ctx)
			return visitNumericLiteralNegative(neg_literal_ctx);
		return visitNumericLiteralUnsigned(ctx->numericLiteralUnsigned());
	}

	std::any QueryUpdateVisitor::visitNumericLiteralUnsigned(sparql_parser::base::SparqlParser::NumericLiteralUnsignedContext *ctx) {
		auto number = ctx->getText();
		if (ctx->DECIMAL())
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Decimal>(number));
		else if (ctx->DOUBLE())
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Double>(number));
		else
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Integer>(number));
	}

	std::any QueryUpdateVisitor::visitNumericLiteralPositive(sparql_parser::base::SparqlParser::NumericLiteralPositiveContext *ctx) {
		auto number = ctx->getText();
		if (ctx->DECIMAL_POSITIVE())
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Decimal>(number));
		if (ctx->DOUBLE_POSITIVE())
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Double>(number));
		return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Integer>(number));
	}
	std::any QueryUpdateVisitor::visitNumericLiteralNegative(sparql_parser::base::SparqlParser::NumericLiteralNegativeContext *ctx) {
		auto number = ctx->getText();
		if (ctx->DECIMAL_NEGATIVE())
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Decimal>(number));
		if (ctx->DOUBLE_NEGATIVE())
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Double>(number));
		return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_typed<rdf4cpp::rdf::datatypes::xsd::Integer>(number));
	}

	std::any QueryUpdateVisitor::visitBooleanLiteral(sparql_parser::base::SparqlParser::BooleanLiteralContext *ctx) {
		if (ctx->TRUE())
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_boolean(true));
		return try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal::make_boolean(false));
	}

	std::any QueryUpdateVisitor::visitString(sparql_parser::base::SparqlParser::StringContext *ctx) {
		auto value = ctx->getText();
		if (ctx->STRING_LITERAL1() or ctx->STRING_LITERAL2())
			return value.substr(1, value.size() - 2);
		return value.substr(3, value.size() - 6);
	}

	std::any QueryUpdateVisitor::visitIri(sparql_parser::base::SparqlParser::IriContext *ctx) {
		if (ctx->IRIREF()) {
			auto iri = ctx->IRIREF()->getText();
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::IRI(iri.substr(1, iri.size() - 2)));
		}
		std::string predicate = ctx->prefixedName()->PNAME_LN()->getText();
		std::size_t split = predicate.find(':');
		try {
			return try_get_in_node_storage_else_default(rdf4cpp::rdf::IRI(prefixes_ptr_->at(predicate.substr(0, split)) + predicate.substr(split + 1)));
		} catch (...) {
			throw MALFORMED_QUERY_EXCEPTION(ctx, "Prefix " + predicate.substr(0, split) + " is not declared.");
		}
	}

	std::any QueryUpdateVisitor::visitBlankNode(sparql_parser::base::SparqlParser::BlankNodeContext *ctx) {
		// todo: we need to throw malformed exception if a blank node is used in different bgps (tricky with the rewriting)
		// todo: use rdf4cpp bnode manager for anonymous variables
		rdf4cpp::rdf::query::Variable var;
		if (auto blank_node_label_ctx = ctx->BLANK_NODE_LABEL(); blank_node_label_ctx)
			var = variable_from_blank_node(blank_node_label_ctx->getText().substr(2));
		var = variable_from_blank_node("bn_" + std::to_string(bnode_anon_counter_++));
		parsed_query_ptr_->register_variable(var);
		return var;
	}

	/* misc */

	rdf4cpp::rdf::query::Variable QueryUpdateVisitor::variable_from_blank_node(std::string const &blank_node_label) const noexcept {
		auto var = rdf4cpp::rdf::query::Variable(blank_node_label, true);
		parsed_query_ptr_->register_variable(var);
		return var;
	}

	expressions::SPARQLExpressionWrapper QueryUpdateVisitor::check_for_alias(rdf4cpp::rdf::query::Variable variable) const noexcept {
		auto found = aliases_.find(variable);
		if (found != aliases_.end())
			return found->second;
		// assign an id to var
		parsed_query_ptr_->register_variable(variable);
		// variables appearing in expressions need be tracked to allow the expressions to be evaluated during query evaluation
		parsed_query_ptr_->track_variable(variable);
		return SPARQLExpressionWrapper{std::make_unique<PrimaryVarExpression>(variable, parsed_query_ptr_->tracked_variable_position(variable))};
	}

	/* operand dependency graph construction */

	detail::operand_desc QueryUpdateVisitor::create_operand_from_tp(detail::OperandDependencyGraph &odg) {
		std::vector<SPARQLQuery::VarID> variable_ids{};
		auto get_slice_key_part = [&](auto const &active_resource) -> std::optional<rdf4cpp::rdf::Node> {
			if (active_resource.is_variable()) {
				auto const var = active_resource.as_variable();
				current_scope_.insert(var);
				vars_in_scope_.insert(var);
				variable_ids.push_back(parsed_query_ptr_->variable_id(var));
				return std::nullopt;
			} else {
				return active_resource;
			}
		};

		detail::SliceKey slice_key{
				get_slice_key_part(active_subject_),
				get_slice_key_part(active_predicate_),
				get_slice_key_part(active_object_)};

		return odg.add_operand(variable_ids, std::make_shared<detail::SliceKey>(std::move(slice_key)));
	}

	std::optional<detail::operand_desc> QueryUpdateVisitor::create_operand_from_filter(detail::OperandDependencyGraph &odg,
																					   std::unique_ptr<SPARQLExpression> expression) const {
		// equality filter optimization
		if (auto equals_expr = dynamic_cast<EqualsExpression *>(expression.get()); equals_expr) {
			auto lhs_op = equals_expr->lhs_expression();
			auto rhs_op = equals_expr->rhs_expression();
			// (?var = term)
			if (dynamic_cast<PrimaryVarExpression const *>(lhs_op.get()) and
				(dynamic_cast<PrimaryLiteralExpression const *>(rhs_op.get()) or
				 dynamic_cast<PrimaryIRIExpression const *>(rhs_op.get()))) {
				auto term = rhs_op->evaluate();
				// if term is literal, we should proceed with normal filter operations
				// example: 5 == 5.0, but the join would fail
				if (not term.is_literal()) {
					auto var = lhs_op->variables()[0];
					if (var.is_anonymous())// always unbound variable in equality filter, trivially false
						return odg.add_filter({}, detail::FilterExpression(std::make_unique<FalseExpression>()));
					auto var_id = parsed_query_ptr_->variable_id(var);
					detail::BoolHypertrie data{1, parsed_query_ptr_->tensor_context().get()};
					data.set({term}, true);
					auto id = parsed_query_ptr_->add_inline_data(std::move(data));
					return odg.add_inline_data({var_id}, id);
				}
			}
			// (term = ?var)
			if (dynamic_cast<PrimaryVarExpression const *>(rhs_op.get()) and
				(dynamic_cast<PrimaryLiteralExpression const *>(lhs_op.get()) or
				 dynamic_cast<PrimaryIRIExpression const *>(lhs_op.get()))) {
				auto term = lhs_op->evaluate();
				// if term is literal, we should proceed with normal filter operations
				// example: 5 == 5.0, but the join would fail
				if (not term.is_literal()) {
					auto var = rhs_op->variables()[0];
					if (var.is_anonymous())// always unbound variable in equality filter, trivially false
						return odg.add_filter({}, detail::FilterExpression(std::make_unique<FalseExpression>()));
					auto var_id = parsed_query_ptr_->variable_id(var);
					detail::BoolHypertrie data{1, parsed_query_ptr_->tensor_context().get()};
					data.set({term}, true);
					auto id = parsed_query_ptr_->add_inline_data(std::move(data));
					return odg.add_inline_data({var_id}, id);
				}
			}
			// (?var = ?var)
			// for equi-joins captured by filter expressions, we replace the variable ids in the operands
			// in order to keep track of the value of both variables we need to create an assignment
			// the lhs is used as the expression of the assignment
			// the rhs is used as the variable to be assigned the value of the assignment
			if (dynamic_cast<PrimaryVarExpression const *>(lhs_op.get()) and dynamic_cast<PrimaryVarExpression const *>(rhs_op.get())) {
				auto lhs_var = lhs_op->variables()[0];
				auto rhs_var = rhs_op->variables()[0];
				if (lhs_var.is_anonymous() or rhs_var.is_anonymous())// always unbound variable in equality filter, trivially false
					return odg.add_filter({}, detail::FilterExpression(std::make_unique<FalseExpression>()));
				auto assignment_var_id = parsed_query_ptr_->variable_id(rhs_var);
				auto assignment_expr_var_id = parsed_query_ptr_->variable_id(lhs_var);
				auto assignment_expr = std::move(lhs_op);
				std::vector<SPARQLQuery::VarID> expr_vars_ids{};
				expr_vars_ids.push_back(assignment_var_id);
				expr_vars_ids.push_back(assignment_expr_var_id);
				if (equality_filter_var_replacement_.contains(assignment_expr_var_id))
					equality_filter_var_replacement_[assignment_var_id] = equality_filter_var_replacement_[assignment_expr_var_id];
				else
					equality_filter_var_replacement_[assignment_var_id] = assignment_expr_var_id;
				return odg.add_assignment(expr_vars_ids, detail::AssignmentExpression(std::move(assignment_expr), assignment_var_id));
			}
		}
		std::vector<SPARQLQuery::VarID> variable_ids{};
		for (auto const &variable : expression->variables()) {
			variable_ids.push_back(parsed_query_ptr_->variable_id(variable));
		}
		return odg.add_filter(variable_ids, detail::FilterExpression(std::move(expression)));
	}

	void QueryUpdateVisitor::create_dependencies_between_operands(detail::OperandDependencyGraph &odg,
																  std::vector<detail::operand_desc> const &descriptors) const {
		for (size_t i = 0; i < descriptors.size() - 1; i++) {
			for (size_t j = i + 1; j < descriptors.size(); j++) {
				odg.add_dependencies(i, j, true);
			}
		}
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::combine_and_merge(detail::OperandDependencyGraph &first_odg,
																		 detail::OperandDependencyGraph &second_odg,
																		 bool bidirectional) const {
		if (first_odg.size() == 0 and second_odg.size() == 0)
			return {};
		// if one of the two provided graphs are empty, do not run the algorithm (we can just return)
		if (first_odg.size() == 0)
			return second_odg;
		if (second_odg.size() == 0)
			return first_odg;
		detail::OperandDependencyGraph new_graph{};
		for (auto &first_union_comp : first_odg.union_components()) {
			auto offset = first_union_comp.size();
			for (auto &second_union_comp : second_odg.union_components()) {
				auto temp_graph = detail::OperandDependencyGraph::merge_graphs(first_union_comp, second_union_comp);
				for (auto first_isc_operand : first_union_comp.isc_operands()) {
					for (auto second_isc_operand : second_union_comp.isc_operands()) {
						temp_graph.add_dependencies(first_isc_operand, second_isc_operand + offset, bidirectional);
					}
				}
				new_graph = detail::OperandDependencyGraph::merge_graphs(new_graph, temp_graph);
			}
		}
		return new_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::combine_optional(detail::OperandDependencyGraph &first_odg,
																		detail::OperandDependencyGraph &second_odg) const {
		if (first_odg.size() == 0 and second_odg.size() == 0)
			return {};
		// if one of the two provided graphs are empty, do not run the algorithm (we can just return)
		if (first_odg.size() == 0)
			return second_odg;
		if (second_odg.size() == 0)
			return first_odg;
		detail::OperandDependencyGraph new_graph{};
		for (auto &first_union_comp : first_odg.union_components()) {
			auto temp_graph = detail::OperandDependencyGraph::merge_graphs(first_union_comp, second_odg);
			size_t offset = first_union_comp.size();
			for (auto &second_union_comp : second_odg.union_components()) {
				for (auto first_isc_op : first_union_comp.isc_operands()) {
					for (auto second_isc_op : second_union_comp.isc_operands()) {
						temp_graph.add_dependencies(first_isc_op, second_isc_op + offset, false);
					}
				}
			}
			new_graph = detail::OperandDependencyGraph::merge_graphs(new_graph, temp_graph);
		}
		return new_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::optional_cartesian_connections(detail::OperandDependencyGraph &first_odg,
																					  detail::OperandDependencyGraph &second_odg) const {
		if (first_odg.size() == 0 and second_odg.size() == 0)
			return {};
		// if one of the two provided graphs are empty, do not run the algorithm (we can just return)
		if (first_odg.size() == 0)
			return second_odg;
		if (second_odg.size() == 0)
			return first_odg;
		// creates optional connections between differnt optional patterns that appear in the same level
		// needs to access the union components in case of patterns like the following { P OPTIONAL { P1 } OPTIONAL { {P2} UNION {P3} } }
		// connections between P1,P2 and P1,P3. NOTE: no connections between P2,P3, thus preserving the UNION
		auto const &first_union_comps = first_odg.union_components();
		auto const &second_union_comps = second_odg.union_components();
		auto merged_graph = detail::OperandDependencyGraph::merge_graphs(first_odg, second_odg);
		size_t offset = first_odg.size();
		for (auto &first_union_comp : first_union_comps) {
			for (auto &second_union_comp : second_union_comps) {
				for (auto first_isc_op : first_union_comp.isc_operands()) {
					for (auto second_isc_op : second_union_comp.isc_operands()) {
						merged_graph.add_connection(first_isc_op, second_isc_op + offset);
						merged_graph.add_connection(second_isc_op + offset, first_isc_op);
					}
				}
				offset += second_union_comp.size();
			}
		}
		return merged_graph;
	}

	detail::OperandDependencyGraph QueryUpdateVisitor::group_graph_pattern_sub_to_odg(GroupGraphPatternSub const &group_graph_pattern_sub) {
		using TriplesBlockContextPtr = sparql_parser::base::SparqlParser::TriplesBlockContext *;
		// for the reordering of group graph patterns, we assume that queries are well-designed
		detail::OperandDependencyGraph operand_dependency_graph{};
		for (auto triples_block_bind_ctx : group_graph_pattern_sub.triples_block_bind_ctxs) {
			if (std::holds_alternative<TriplesBlockContextPtr>(triples_block_bind_ctx)) {
				auto temp_graph = (visit_triples_block(std::get<0>(triples_block_bind_ctx)));
				operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
			} else {
				auto temp_graph = (visit_bind(std::get<1>(triples_block_bind_ctx)));
				operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
			}
		}
		for (auto inline_data_ctx : group_graph_pattern_sub.inline_ctxs) {
			auto temp_graph = (visit_inline_data(inline_data_ctx));
			operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
		}
		for (auto group_or_union_ctx : group_graph_pattern_sub.group_or_union_ctxs) {
			auto temp_graph = (visit_group_or_union_graph_pattern(group_or_union_ctx));
			operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
		}
		detail::OperandDependencyGraph optional_cartesian_odg{};
		for (auto optional_ctx : group_graph_pattern_sub.optional_ctxs) {
			auto temp_graph = (visit_optional_graph_pattern(optional_ctx));
			optional_cartesian_odg = optional_cartesian_connections(optional_cartesian_odg, temp_graph);
		}
		// we need to visit optional before filter in order to gather the variables that are in scope
		equality_filter_var_replacement_.clear();
		for (auto filter_ctx : group_graph_pattern_sub.filter_ctxs) {
			auto temp_graph = (visit_filter(filter_ctx));
			operand_dependency_graph = combine_and_merge(operand_dependency_graph, temp_graph);
		}
		// combine and merge the optional part last (ensures dependencies between filters and optional parts)
		operand_dependency_graph = combine_optional(operand_dependency_graph, optional_cartesian_odg);
		// apply variable replacement from equality filters
		if (not equality_filter_var_replacement_.empty())
			operand_dependency_graph.replace_var_ids_in_operands(equality_filter_var_replacement_);
		return operand_dependency_graph;
	}

	rdf4cpp::rdf::IRI QueryUpdateVisitor::try_get_in_node_storage_else_default(rdf4cpp::rdf::IRI iri,
																			   rdf4cpp::rdf::storage::node::NodeStorage &default_storage) const {
		auto tried_node = iri.try_get_in_node_storage(node_storage_);
		if (not tried_node.null())
			return tried_node;
		return iri.to_node_storage(default_storage);
	}

	rdf4cpp::rdf::Literal QueryUpdateVisitor::try_get_in_node_storage_else_default(rdf4cpp::rdf::Literal literal,
																				   rdf4cpp::rdf::storage::node::NodeStorage &default_storage) const {
		auto tried_node = literal.try_get_in_node_storage(node_storage_);
		if (not tried_node.null())
			return tried_node;
		return literal.to_node_storage(default_storage);
	}


}// namespace dice::sparql::parser::visitors