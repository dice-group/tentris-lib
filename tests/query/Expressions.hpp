#ifndef HYPERTRIE_EXPRESSIONS_HPP
#define HYPERTRIE_EXPRESSIONS_HPP

#include <dice/query.hpp>

namespace dice::query::tests {

	using htt_t = hypertrie::default_bool_Hypertrie_trait;
	using allocator_type = std::allocator<std::byte>;

	class Binding : public Expression<htt_t, allocator_type> {
	private:
		size_t position_;
		typename htt_t::key_part_type value_{};

	public:
		explicit Binding(size_t position) : position_(position) {}
		void update_value(CountedKey<htt_t> const &key) override { value_ = key[position_]; }
		[[nodiscard]] typename htt_t::key_part_type evaluate() const override { return value_; }
		[[nodiscard]] typename htt_t::key_part_type evaluate([[maybe_unused]] EvaluationContext<htt_t, allocator_type> &ctx) const override { return evaluate(); }
	protected:
		[[nodiscard]]  Binding *clone_impl() const override { return new Binding(*this); }
	};

	std::unique_ptr<Binding> gen_binding(size_t pos) { return std::make_unique<Binding>(pos); }

	void track_and_bindings(Query<htt_t, allocator_type> &query, size_t num) {
		char var = 'a';
		for (size_t i = 0; i < num; i++) {
			query.track_variable(var);
            query.add_binding(ExpressionWrapper<htt_t, allocator_type>{gen_binding(i)});
            var++;
		}
	}

	class TrueExpression : public Expression<htt_t, allocator_type> {
	public:
		TrueExpression() = default;
		void update_value([[maybe_unused]] CountedKey<htt_t> const &key) override {}
		[[nodiscard]] typename htt_t::key_part_type evaluate() const override { return 1; }
		[[nodiscard]] typename htt_t::key_part_type evaluate([[maybe_unused]] EvaluationContext<htt_t, allocator_type> &ctx) const override { return evaluate(); }
	protected:
		[[nodiscard]]  TrueExpression *clone_impl() const override { return new TrueExpression(*this); }
	};

	class FalseExpression : public Expression<htt_t, allocator_type> {
	public:
		FalseExpression() = default;
		void update_value([[maybe_unused]] CountedKey<htt_t> const &key) override {}
		[[nodiscard]] typename htt_t::key_part_type evaluate() const override { return 0; }
		[[nodiscard]] typename htt_t::key_part_type evaluate([[maybe_unused]] EvaluationContext<htt_t, allocator_type> &ctx) const override { return evaluate(); }
	protected:
		[[nodiscard]]  FalseExpression *clone_impl() const override { return new FalseExpression(*this); }
	};

	class SquareExpression : public Expression<htt_t, allocator_type> {
	private:
		std::unique_ptr<Expression<htt_t, allocator_type>> expression_;
	public:
		explicit SquareExpression(std::unique_ptr<Expression<htt_t, allocator_type>> expression) : expression_(std::move(expression)) {}
		void update_value(CountedKey<htt_t> const &key) override { expression_->update_value(key); }
		[[nodiscard]] typename htt_t::key_part_type evaluate() const override { return expression_->evaluate()*expression_->evaluate(); }
		[[nodiscard]] typename htt_t::key_part_type evaluate([[maybe_unused]] EvaluationContext<htt_t, allocator_type> &ctx) const override { return evaluate(); }
	protected:
		[[nodiscard]]  SquareExpression *clone_impl() const override { return new SquareExpression(expression_->clone()); }
	};

}// namespace Dice::query::tests

#endif//HYPERTRIE_EXPRESSIONS_HPP
