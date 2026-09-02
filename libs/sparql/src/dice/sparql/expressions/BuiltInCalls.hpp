#ifndef TENTRIS_QUERY_SPARQL_BUILTINCALLS_HPP
#define TENTRIS_QUERY_SPARQL_BUILTINCALLS_HPP

#include <array>
#include <vector>
#include <memory>

#include <dice/sparql/expressions/Expression.hpp>

namespace dice::sparql::expressions {

	/**
	 * Can be used as a template parameter for NArySparqlExpression
	 * to make it an SPARQLExpression of arbitrary argument count, i.e. not fixed by a constant.
	 * This changes the internal representation of the operands from std::array to std::vector
	 */
	inline constexpr size_t DYN_EXPR_SIZE = std::dynamic_extent;

	template<size_t N>
	class NArySPARQLExpression : public SPARQLExpression {
	public:
		using Inner = std::conditional_t<N == DYN_EXPR_SIZE,
										 std::vector<std::unique_ptr<SPARQLExpression>>,
										 std::array<std::unique_ptr<SPARQLExpression>, N>>;

	protected:
		Inner op_exprs_;

		template<typename T>
		[[nodiscard]] T *generic_clone() const;

	public:
		explicit NArySPARQLExpression(Inner &&op_exprs) noexcept;

		template<std::same_as<SPARQLExpressionWrapper> ...Exprs> requires (sizeof...(Exprs) == N)
		explicit NArySPARQLExpression(Exprs ...exprs) noexcept : op_exprs_{std::move(exprs)...} {
		}

		void update_value(detail::Key const &key) override;

		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> aggregated_variables() const override;
		[[nodiscard]] std::vector<rdf4cpp::rdf::query::Variable> non_aggregated_variables() const override;
	};

	extern template class NArySPARQLExpression<1>;
	extern template class NArySPARQLExpression<2>;
	extern template class NArySPARQLExpression<3>;
	extern template class NArySPARQLExpression<4>;
	extern template class NArySPARQLExpression<DYN_EXPR_SIZE>;


	/* https://www.w3.org/TR/sparql11-query/#func-isiri */
	class IsIRI : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] IsIRI *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-iri */
	class IRI : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] IRI *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-isblank */
	class IsBlank : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] IsBlank *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-isliteral */
	class IsLiteral : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] IsLiteral *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-isnumeric */
	class IsNumeric : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] IsNumeric *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-abs */
	class Abs : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Abs *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-ceil */
	class Ceil : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Ceil *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-floor */
	class Floor : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Floor *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-round */
	class Round : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Round *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-md5 */
	class MD5 : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] MD5 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-sha1 */
	class SHA1 : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] SHA1 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-sha256 */
	class SHA256 : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] SHA256 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-sha384 */
	class SHA384 : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] SHA384 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-sha512 */
	class SHA512 : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] SHA512 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-datatype */
	class Datatype : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Datatype *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-sameTerm */
	class SameTerm : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] SameTerm *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strdt */
	class StrDt : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] StrDt *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-str */
	class Str : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Str *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-contains */
	class Contains : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Contains *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-concat */
	class Concat : public NArySPARQLExpression<DYN_EXPR_SIZE> {
	public:
		using NArySPARQLExpression<DYN_EXPR_SIZE>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Concat *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strstarts */
	class StrStarts : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] StrStarts *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strbefore */
	class StrBefore : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] StrBefore *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strafter */
	class StrAfter : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] StrAfter *clone_impl() const override;
	};


	/* https://www.w3.org/TR/sparql11-query/#func-strends */
	class StrEnds : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] StrEnds *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strlen */
	class StrLen : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] StrLen *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-ucase */
	class UCase : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] UCase *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-lcase */
	class LCase : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] LCase *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-encode */
	class EncodeForURI : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] EncodeForURI *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-substr (2 argument version) */
	class SubStr2 : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] SubStr2 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-substr (3 argument version) */
	class SubStr3 : public NArySPARQLExpression<3> {
	public:
		using NArySPARQLExpression<3>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] SubStr3 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-langMatches */
	class LangMatches : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] LangMatches *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-lang */
	class Lang : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Lang *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-strlang */
	class StrLang : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] StrLang *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-bound */
	class Bound : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Bound *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-uuid */
	class UUID : public NArySPARQLExpression<0> {
	public:
		UUID() noexcept = default;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] UUID *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-struuid */
	class StrUUID : public NArySPARQLExpression<0> {
	public:
		StrUUID() noexcept = default;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] StrUUID *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-rand */
	class Rand : public NArySPARQLExpression<0> {
	public:
		Rand() noexcept = default;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Rand *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-now */
	class Now : public NArySPARQLExpression<0> {
	public:
		Now() noexcept = default;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate(detail::EvaluationContext &eval_ctx) const override;
	protected:
		[[nodiscard]] Now *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-year */
	class Year : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Year *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-month */
	class Month : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Month *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-day */
	class Day : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Day *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-hours */
	class Hours : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Hours *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-minutes */
	class Minutes : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Minutes *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-seconds */
	class Seconds : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Seconds *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-timezone */
	class Timezone : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Timezone *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-timezone */
	class TZ : public NArySPARQLExpression<1> {
	public:
		using NArySPARQLExpression<1>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] TZ *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-if */
	class If : public NArySPARQLExpression<3> {
	public:
		using NArySPARQLExpression<3>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] If *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-coalesce */
	class Coalesce : public NArySPARQLExpression<DYN_EXPR_SIZE> {
	public:
		using NArySPARQLExpression<DYN_EXPR_SIZE>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Coalesce *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-regex (2 argument version) */
	class Regex2 : public NArySPARQLExpression<2> {
	public:
		using NArySPARQLExpression<2>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Regex2 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-regex (3 argument version) */
	class Regex3 : public NArySPARQLExpression<3> {
	public:
		using NArySPARQLExpression<3>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Regex3 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-replace (3 argument version) */
	class Replace3 : public NArySPARQLExpression<3> {
	public:
		using NArySPARQLExpression<3>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Replace3 *clone_impl() const override;
	};

	/* https://www.w3.org/TR/sparql11-query/#func-replace (4 argument version) */
	class Replace4 : public NArySPARQLExpression<4> {
	public:
		using NArySPARQLExpression<4>::NArySPARQLExpression;
		[[nodiscard]] node_wrapper::NodeWrapper evaluate() const override;
	protected:
		[[nodiscard]] Replace4 *clone_impl() const override;
	};

}//namespace dice::sparql2tensor::expressions

#endif//TENTRIS_QUERY_SPARQL_BUILTINCALLS_HPP
