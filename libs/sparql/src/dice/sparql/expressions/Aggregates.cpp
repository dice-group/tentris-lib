#include "Aggregates.hpp"

#include <utility>

namespace dice::sparql::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	/* Aggregate Expression */
	Aggregate::Aggregate(std::unique_ptr<SPARQLExpression> op_expr)
		: op_expr_(std::move(op_expr)) {}

	[[nodiscard]] std::vector<Variable> Aggregate::variables() const {
		return op_expr_->variables();
	}

	[[nodiscard]] std::vector<Variable> Aggregate::aggregated_variables() const {
		return op_expr_->variables();
	}

	[[nodiscard]] std::vector<Variable> Aggregate::non_aggregated_variables() const {
		return {};
	}

	/* CountStar Expression */
	CountStar::CountStar(size_t count)
		: Aggregate(nullptr), count_(count) {}

	void CountStar::update_value([[maybe_unused]] detail::Key const &key) {
		count_++;
	}

	node_wrapper::NodeWrapper CountStar::evaluate() const {
		return Literal::make_typed_from_value<rdf4cpp::rdf::datatypes::xsd::Integer>(count_);
	}

	CountStar *CountStar::clone_impl() const {
		return new CountStar(count_);
	}

	[[nodiscard]] std::vector<Variable> CountStar::variables() const {
		return {};
	}

	/* CountStarDistinct Expression */
	CountStarDistinct::CountStarDistinct(boost::container::flat_set<size_t> keys)
		: Aggregate(nullptr), keys_(std::move(keys)) {}

	void CountStarDistinct::update_value(detail::Key const &key) {
		keys_.insert(dice::hash::dice_hash_templates<dice::hash::Policies::wyhash>::dice_hash(key));
	}

	node_wrapper::NodeWrapper CountStarDistinct::evaluate() const {
		return Literal::make_typed_from_value<rdf4cpp::rdf::datatypes::xsd::Integer>(keys_.size());
	}

	CountStarDistinct *CountStarDistinct::clone_impl() const {
		return new CountStarDistinct(keys_);
	}

	[[nodiscard]] std::vector<Variable> CountStarDistinct::variables() const {
		return {};
	}

	/* Count Expression */
	Count::Count(std::unique_ptr<SPARQLExpression> expr, size_t count)
		: Aggregate(std::move(expr)), count_(count) {}

	void Count::update_value(detail::Key const &key) {
		op_expr_->update_value(key);
		auto expr_result = op_expr_->evaluate();
		if (not expr_result.null())
			count_++;
	}

	node_wrapper::NodeWrapper Count::evaluate() const {
		return Literal::make_typed_from_value<rdf4cpp::rdf::datatypes::xsd::Integer>(count_);
	}

	Count *Count::clone_impl() const {
		return new Count(op_expr_->clone(), count_);
	}

	/* CountDistinct Expression */
	CountDistinct::CountDistinct(std::unique_ptr<SPARQLExpression> expr, boost::container::flat_set<Node> rdf_nodes)
		: Aggregate(std::move(expr)), rdf_nodes_(std::move(rdf_nodes)) {}

	void CountDistinct::update_value([[maybe_unused]] detail::Key const &key) {
		op_expr_->update_value(key);
		auto expr_result = op_expr_->evaluate();
		if (not expr_result.null())
			rdf_nodes_.insert(expr_result);
	}

	node_wrapper::NodeWrapper CountDistinct::evaluate() const {
		return Literal::make_typed_from_value<rdf4cpp::rdf::datatypes::xsd::Integer>(rdf_nodes_.size());
	}

	CountDistinct *CountDistinct::clone_impl() const {
		return new CountDistinct(op_expr_->clone(), rdf_nodes_);
	}

	/* Sum Expression */
	Sum::Sum(std::unique_ptr<SPARQLExpression> expr, Literal rdf_literal)
		: Aggregate(std::move(expr)), rdf_literal_(rdf_literal) {}

	void Sum::update_value(detail::Key const &key) {
		if (rdf_literal_.null())
			return;
		op_expr_->update_value(key);
		rdf_literal_ = rdf_literal_ + op_expr_->evaluate().as_literal();
	}

	node_wrapper::NodeWrapper Sum::evaluate() const {
		return rdf_literal_;
	}

	Sum *Sum::clone_impl() const {
		return new Sum(op_expr_->clone(), rdf_literal_);
	}

	/* SumDistinct Expression */
	SumDistinct::SumDistinct(std::unique_ptr<SPARQLExpression> expr, Literal rdf_literal, boost::container::flat_set<Literal> rdf_literals)
		: Aggregate(std::move(expr)), rdf_literal_(rdf_literal), rdf_literals_(std::move(rdf_literals)) {}

	void SumDistinct::update_value(detail::Key const &key) {
		if (rdf_literal_.null())
			return;
		op_expr_->update_value(key);
		auto expr_result_lit = op_expr_->evaluate().as_literal();
		if (rdf_literals_.contains(expr_result_lit.as_literal()))
			return;
		rdf_literal_ = rdf_literal_ + expr_result_lit;
	}

	node_wrapper::NodeWrapper SumDistinct::evaluate() const {
		return rdf_literal_;
	}

	SumDistinct *SumDistinct::clone_impl() const {
		return new SumDistinct(op_expr_->clone(), rdf_literal_, rdf_literals_);
	}

	/* Avg Expression */
	Avg::Avg(std::unique_ptr<SPARQLExpression> expr, bool distinct)
		: Aggregate(std::move(expr)) {
		if (not distinct) {
			sum_expr_ = std::make_unique<Sum>(op_expr_->clone());
			count_expr_ = std::make_unique<Count>(op_expr_->clone());
		} else {
			sum_expr_ = std::make_unique<SumDistinct>(op_expr_->clone());
			count_expr_ = std::make_unique<CountDistinct>(op_expr_->clone());
		}
	}

	Avg::Avg(std::unique_ptr<SPARQLExpression> expr, std::unique_ptr<SPARQLExpression> sum_expr, std::unique_ptr<SPARQLExpression> count_expr)
		: Aggregate(std::move(expr)), sum_expr_(std::move(sum_expr)), count_expr_(std::move(count_expr)) {}

	void Avg::update_value(detail::Key const &key) {
		sum_expr_->update_value(key);
		count_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper Avg::evaluate() const {
		auto count_result_lit = count_expr_->evaluate().as_literal();
		if (std::any_cast<rdf4cpp::rdf::datatypes::xsd::Integer::cpp_type>(count_result_lit.value()) == 0)
			return rdf4cpp::rdf::Literal::make_typed_from_value<rdf4cpp::rdf::datatypes::xsd::Integer>(0);
		auto sum_result_lit = sum_expr_->evaluate().as_literal();
		if (sum_result_lit.null())
			return {};
		return sum_result_lit / count_result_lit;
	}

	Avg *Avg::clone_impl() const {
		return new Avg(op_expr_->clone(), sum_expr_->clone(), count_expr_->clone());
	}

	/* Min Expression */
	Min::Min(std::unique_ptr<SPARQLExpression> expr, Node rdf_node)
		: Aggregate(std::move(expr)), rdf_node_(rdf_node) {}

	void Min::update_value([[maybe_unused]] detail::Key const &key) {
		op_expr_->update_value(key);
		auto expr_result = op_expr_->evaluate();
		if (expr_result.null())
			return;
		if (static_cast<Node>(expr_result) < rdf_node_ or rdf_node_.null())
			rdf_node_ = expr_result;
	}

	node_wrapper::NodeWrapper Min::evaluate() const {
		return rdf_node_;
	}

	Min *Min::clone_impl() const {
		return new Min(op_expr_->clone(), rdf_node_);
	}

	/* Max Expression */
	Max::Max(std::unique_ptr<SPARQLExpression> expr, Node rdf_node)
		: Aggregate(std::move(expr)), rdf_node_(rdf_node) {}

	void Max::update_value([[maybe_unused]] detail::Key const &key) {
		op_expr_->update_value(key);
		auto expr_result = op_expr_->evaluate();
		if (expr_result.null())
			return;
		if (static_cast<Node>(expr_result) > rdf_node_)
			rdf_node_ = expr_result;
	}

	node_wrapper::NodeWrapper Max::evaluate() const {
		return rdf_node_;
	}

	Max *Max::clone_impl() const {
		return new Max(op_expr_->clone(), rdf_node_);
	}

	/* Sample Expression */
	Sample::Sample(std::unique_ptr<SPARQLExpression> expr)
		: Aggregate(std::move(expr)) {}

	void Sample::update_value([[maybe_unused]] detail::Key const &key) {
		op_expr_->update_value(key);
	}

	node_wrapper::NodeWrapper Sample::evaluate() const {
		return op_expr_->evaluate();
	}

	Sample *Sample::clone_impl() const {
		return new Sample(op_expr_->clone());
	}


}// namespace dice::sparql::expressions