#ifndef QUERY_EXPRESSION_HPP
#define QUERY_EXPRESSION_HPP

#include <dice/query/Commons.hpp>

namespace dice::query {

	/**
	 * @brief An abstract interface of an expression.
	 * @tparam htt_t The hypertrie trait. Only boolean traits are supported.
	 */
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	class Expression {
	public:
		Expression() = default;
		virtual ~Expression() = default;
		/* updates the value of the expression using the provided entry */
		virtual void update_value(const CountedKey<htt_t> &key) = 0;
		/* returns the result of the expression */
		[[nodiscard]] virtual typename htt_t::key_part_type evaluate() const = 0;
		[[nodiscard]] virtual typename htt_t::key_part_type evaluate(EvaluationContext<htt_t, allocator_type> &eval_ctx) const = 0;
		/* deep copy of the expression */
		[[nodiscard]] std::unique_ptr<Expression> clone() const { return std::unique_ptr<Expression>(clone_impl()); }
	protected:
		[[nodiscard]] virtual Expression *clone_impl() const = 0;
	};

	/**
	 * @brief A wrapper for Expressions.
	 * Encapsulates an expression in a unique_ptr, thus enabling polymorphism.
	 * @tparam htt_t The hypertrie trait. Only boolean traits are supported.
	 */
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	class ExpressionWrapper {
	private:
		mutable std::unique_ptr<Expression<htt_t, allocator_type>> expression_;

	public:
		ExpressionWrapper() = delete;

		explicit ExpressionWrapper(std::unique_ptr<Expression<htt_t, allocator_type>> expression)
			: expression_(std::move(expression)) {}

		ExpressionWrapper(ExpressionWrapper const &other)
			: expression_(other.expression_->clone()) {}

		ExpressionWrapper(ExpressionWrapper &&other) noexcept = default;

		ExpressionWrapper &operator=(ExpressionWrapper const &other) {
			expression_ = other.expression_->clone();
			return *this;
		}

		ExpressionWrapper &operator=(ExpressionWrapper &&other) noexcept = default;

		~ExpressionWrapper() = default;

		void update_value(CountedKey<htt_t> const &key) const { expression_->update_value(key); }

		[[nodiscard]] typename htt_t::key_part_type evaluate() const { return expression_->evaluate(); }

		[[nodiscard]] typename htt_t::key_part_type evaluate(EvaluationContext<htt_t, allocator_type> &eval_ctx) const { return expression_->evaluate(eval_ctx); }

        operator std::unique_ptr<Expression<htt_t, allocator_type>>() && {
            return std::move(expression_);
        }
	};

	/**
	 * @brief A wrapper for expressions used in filter expressions.
	 * It encapsulates an expression, thus enabling polymorphism
	 * and allowing the expression to be shared (copied) during the query evaluation
	 * @tparam htt_t The hypertrie trait. Only boolean traits are supported.
	 */
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	class FilterExpression {
	protected:
		std::shared_ptr<Expression<htt_t, allocator_type>> expression_ = nullptr;

	public:
		FilterExpression() = default;

		explicit FilterExpression(std::unique_ptr<Expression<htt_t, allocator_type>> expression)
			: expression_(std::move(expression)) {}

		void update_value(CountedKey<htt_t> const &key) { expression_->update_value(key); }

		[[nodiscard]] bool evaluate() const {
			return bool(expression_->evaluate());
		}

		[[nodiscard]] bool evaluate(EvaluationContext<htt_t, allocator_type> &eval_ctx) const {
			return bool(expression_->evaluate(eval_ctx));
		}

		[[nodiscard]] bool is_null() const { return expression_ == nullptr; }

	};

	/**
	 * @brief A representation of (var = expr) expressions. Evaluates the provided expressions (expr) and holds the var_id
	 * of the provided variable (var). Evaluated in the AssignmentOperator
	 * @tparam htt_t The hypertrie trait. Only boolean traits are supported.
	 */
	template<hypertrie::HypertrieTrait_bool_valued htt_t, hypertrie::ByteAllocator allocator_type>
	class AssignmentExpression {
	protected:
		std::shared_ptr<Expression<htt_t, allocator_type>> expression_ = nullptr;
		char result_var_id_ = '\0';

	public:
		AssignmentExpression() = default;

		explicit AssignmentExpression(std::unique_ptr<Expression<htt_t, allocator_type>> expression, char result_var_id)
			: expression_(std::move(expression)), result_var_id_(result_var_id) {}

		void update_value(CountedKey<htt_t> const &key) { expression_->update_value(key); }

		[[nodiscard]] typename htt_t::key_part_type evaluate() const { return expression_->evaluate(); }

		[[nodiscard]] typename htt_t::key_part_type evaluate(EvaluationContext<htt_t, allocator_type> &eval_ctx) const { return expression_->evaluate(eval_ctx); }

		[[nodiscard]] char result_var_id() const { return result_var_id_; }

		[[nodiscard]] bool is_null() const { return expression_ == nullptr; }

	};

} // namespace dice::query

#endif//QUERY_EXPRESSION_HPP
