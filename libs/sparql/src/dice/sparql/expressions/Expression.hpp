#ifndef DICE_SPARQL_EXPRESSION_HPP
#define DICE_SPARQL_EXPRESSION_HPP

#include <dice/node-wrapper/NodeWrapper.hpp>
#include <dice/sparql/detail/tensor.hpp>
#include <rdf4cpp/rdf/query/Variable.hpp>

namespace dice::sparql::expressions {

	class SPARQLExpression : public detail::Expression {
	public:
		SPARQLExpression() = default;
		[[nodiscard]] std::unique_ptr<SPARQLExpression> clone() const { return std::unique_ptr<SPARQLExpression>(clone_impl()); }
		[[nodiscard]] virtual node_wrapper::NodeWrapper evaluate() const override = 0;
		[[nodiscard]] virtual node_wrapper::NodeWrapper evaluate([[maybe_unused]] detail::EvaluationContext &eval_ctx) const override { return evaluate(); }
		[[nodiscard]] virtual std::vector<rdf4cpp::rdf::query::Variable> variables() const = 0;
		[[nodiscard]] virtual std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const = 0;
		[[nodiscard]] virtual std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const = 0;

	protected:
		[[nodiscard]] SPARQLExpression *clone_impl() const override = 0;
	};

    /**
	 * @brief A wrapper for <div>SPARQLExpression</div>s.
	 * Encapsulates an SparqlExpression in a unique_ptr, thus enabling polymorphism. Further, copy constructors are defined, thus enabling wrapping into std::any.
	 */
    class SPARQLExpressionWrapper {
    private:
        std::unique_ptr<SPARQLExpression> expression_;

    public:
        SPARQLExpressionWrapper() = delete;

        explicit SPARQLExpressionWrapper(std::unique_ptr<SPARQLExpression> expression)
            : expression_(std::move(expression)) {}

        SPARQLExpressionWrapper(SPARQLExpressionWrapper const &other)
            : expression_(other.expression_->clone()) {}

        SPARQLExpressionWrapper(SPARQLExpressionWrapper &&other) noexcept = default;

        SPARQLExpressionWrapper &operator=(SPARQLExpressionWrapper const &other) {
            expression_ = other.expression_->clone();
            return *this;
        }

        SPARQLExpressionWrapper &operator=(SPARQLExpressionWrapper &&other) noexcept = default;

        ~SPARQLExpressionWrapper() = default;

        void update_value(detail::Key const &key) { expression_->update_value(key); }

        [[nodiscard]] detail::key_part_type evaluate() const { return expression_->evaluate(); }

		[[nodiscard]] detail::key_part_type evaluate(detail::EvaluationContext &eval_ctx) const { return expression_->evaluate(eval_ctx); }

        [[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const {
                return expression_->variables();
        };

		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const {
			return expression_->aggregated_variables();
		};

		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const {
			return expression_->non_aggregated_variables();
		};

        [[nodiscard]] SPARQLExpression const *get() const {
                return expression_.get();
        }

        [[nodiscard]] SPARQLExpression *get() {
                return expression_.get();
        }

        operator detail::ExpressionWrapper() && noexcept {
                return detail::ExpressionWrapper{std::move(expression_)};
        }

        operator std::unique_ptr<SPARQLExpression>() && noexcept {
                return std::move(expression_);
        }
    };

}//namespace dice::sparql2tensor::expressions

#endif//DICE_SPARQL_EXPRESSION_HPP
