#ifndef TENTRIS_QUERY_SPARQL_TENSOR_HPP
#define TENTRIS_QUERY_SPARQL_TENSOR_HPP

#include <dice/query.hpp>
#include <dice/tentris/hypertrie-template-instantiation.hpp>
#include <dice/tentris/param_allocator.hpp>


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnull-pointer-arithmetic"
#include <metall/basic_manager.hpp>
#pragma GCC diagnostic pop


namespace dice::tentris::defs {
	using Key = dice::query::CountedKey<htt_t>;
	using Entry = dice::query::CountedEntry<htt_t>;
	using Query = dice::query::Query<htt_t, allocator_type>;
	using QueryEvaluation = dice::query::Evaluation;
	using EvaluationContext = dice::query::EvaluationContext<htt_t, allocator_type>;
	using operand_desc = dice::query::operand_desc;
	using Expression = dice::query::Expression<htt_t, allocator_type>;
	using FilterExpression = dice::query::FilterExpression<htt_t, allocator_type>;
	using AssignmentExpression = dice::query::AssignmentExpression<htt_t, allocator_type>;
	using ExpressionWrapper = dice::query::ExpressionWrapper<htt_t, allocator_type>;
	using OperandDependencyGraph = dice::query::OperandDependencyGraph<htt_t, allocator_type>;
};// namespace dice::tentris::defs

namespace dice::sparql::detail {
	using namespace dice::tentris::defs;
}// namespace dice::sparql::detail


#endif//TENTRIS_QUERY_SPARQL_TENSOR_HPP
