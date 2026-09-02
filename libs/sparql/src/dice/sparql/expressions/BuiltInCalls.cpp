#include "BuiltInCalls.hpp"

#include <algorithm>

#define IMPL_UNIT_CLONE(type)        \
	type *type::clone_impl() const { \
    	return new type{};           \
	}

#define IMPL_CLONE(type)               \
	type *type::clone_impl() const {   \
		return generic_clone<type>();  \
	}

namespace dice::sparql::expressions {

	using namespace rdf4cpp::rdf;
	using namespace rdf4cpp::rdf::query;

	template<size_t N>
	template<typename T>
	T *NArySPARQLExpression<N>::generic_clone() const {
		Inner copy;
		if constexpr (N == DYN_EXPR_SIZE) {
			copy.resize(op_exprs_.size());
		}

		std::transform(op_exprs_.begin(), op_exprs_.end(), copy.begin(), [](auto const &x) { return x->clone(); });
		return new T{std::move(copy)};
	}

	template<size_t N>
	NArySPARQLExpression<N>::NArySPARQLExpression(Inner &&op_exprs) noexcept : op_exprs_{std::move(op_exprs)} {
	}

	template<size_t N>
	void NArySPARQLExpression<N>::update_value(detail::Key const &key) {
		for (auto &expr : op_exprs_) {
			expr->update_value(key);
		}
	}

	template<size_t N>
	std::vector<rdf4cpp::rdf::query::Variable> NArySPARQLExpression<N>::variables() const {
		std::vector<rdf4cpp::rdf::query::Variable> vars;
		for (auto const &expr : op_exprs_) {
			auto v = expr->variables();
			vars.insert(vars.end(), v.begin(), v.end());
		}

		return vars;
	}

	template<size_t N>
	std::vector<rdf4cpp::rdf::query::Variable> NArySPARQLExpression<N>::aggregated_variables() const {
		std::vector<rdf4cpp::rdf::query::Variable> vars;
		for (auto const &expr : op_exprs_) {
			auto v = expr->aggregated_variables();
			vars.insert(vars.end(), v.begin(), v.end());
		}

		return vars;
	}

	template<size_t N>
	std::vector<rdf4cpp::rdf::query::Variable> NArySPARQLExpression<N>::non_aggregated_variables() const {
		std::vector<rdf4cpp::rdf::query::Variable> vars;
		for (auto const &expr : op_exprs_) {
			auto v = expr->non_aggregated_variables();
			vars.insert(vars.end(), v.begin(), v.end());
		}

		return vars;
	}

	template class NArySPARQLExpression<0>;
	template class NArySPARQLExpression<1>;
	template class NArySPARQLExpression<2>;
	template class NArySPARQLExpression<3>;
	template class NArySPARQLExpression<4>;
	template class NArySPARQLExpression<DYN_EXPR_SIZE>;

	/* IsIRI Expression */
	node_wrapper::NodeWrapper IsIRI::evaluate() const {
		auto expr_result = op_exprs_[0]->evaluate();
		return Literal::make_boolean(expr_result.is_iri());
	}

	IMPL_CLONE(IsIRI)

	/* IRI Expression */
	node_wrapper::NodeWrapper IRI::evaluate() const {
		auto expr_result = op_exprs_[0]->evaluate();

		if (auto iri_result = expr_result.as_iri(); not iri_result.null()) {
			return iri_result;
		}

		if (auto lit_result = expr_result.as_literal(); not lit_result.null() and lit_result.datatype_eq<datatypes::xsd::String>()) {
			return rdf4cpp::rdf::IRI{lit_result.lexical_form()};
		}

		return rdf4cpp::rdf::IRI{};
	}

	IMPL_CLONE(IRI);

	/* IsBlank Expression */
	node_wrapper::NodeWrapper IsBlank::evaluate() const {
		return Literal::make_boolean(op_exprs_[0]->evaluate().is_blank_node());
	}

	IMPL_CLONE(IsBlank);

	/* IsLiteral Expression */
	node_wrapper::NodeWrapper IsLiteral::evaluate() const {
		return Literal::make_boolean(op_exprs_[0]->evaluate().is_literal());
	}

	IMPL_CLONE(IsLiteral);

	/* IsNumeric Expression */
	node_wrapper::NodeWrapper IsNumeric::evaluate() const {
		auto expr_result = op_exprs_[0]->evaluate().as_literal();
		return Literal::make_boolean(not expr_result.null() and expr_result.is_numeric());
	}

	IMPL_CLONE(IsNumeric);

	/* Abs Expression */
	node_wrapper::NodeWrapper Abs::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().abs();
	}

	IMPL_CLONE(Abs);

	/* Ceil Expression */
	node_wrapper::NodeWrapper Ceil::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().ceil();
	}

	IMPL_CLONE(Ceil);

	/* Floor Expression */
	node_wrapper::NodeWrapper Floor::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().floor();
	}

	IMPL_CLONE(Floor);

	/* Round Expression */
	node_wrapper::NodeWrapper Round::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().round();
	}

	IMPL_CLONE(Round);

	/* MD5 Expression */
	node_wrapper::NodeWrapper MD5::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().md5();
	}

	IMPL_CLONE(MD5);

	/* SHA1 Expression */
	node_wrapper::NodeWrapper SHA1::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().sha1();
	}

	IMPL_CLONE(SHA1);

	/* SHA256 Expression */
	node_wrapper::NodeWrapper SHA256::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().sha256();
	}

	IMPL_CLONE(SHA256);

	/* SHA384 Expression */
	node_wrapper::NodeWrapper SHA384::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().sha384();
	}

	IMPL_CLONE(SHA384);

	/* SHA512 Expression */
	node_wrapper::NodeWrapper SHA512::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().sha512();
	}

	IMPL_CLONE(SHA512);

	/* Datatype Expression */
	node_wrapper::NodeWrapper Datatype::evaluate() const {
		auto expr_result_lit = op_exprs_[0]->evaluate().as_literal();
		if (expr_result_lit.null()) {
			return rdf4cpp::rdf::IRI{};
		}

		return expr_result_lit.datatype();
	}

	IMPL_CLONE(Datatype);

	/* SameTerm Expression */
	node_wrapper::NodeWrapper SameTerm::evaluate() const {
		return Literal::make_boolean(op_exprs_[0]->evaluate() == op_exprs_[1]->evaluate());
	}

	IMPL_CLONE(SameTerm);

	/* StrDt Expression */
	node_wrapper::NodeWrapper StrDt::evaluate() const {
		auto lex_form = op_exprs_[0]->evaluate().as_literal();
		if (lex_form.null() or not lex_form.datatype_eq<datatypes::xsd::String>()) {
			return Literal{};
		}

		auto datatype = op_exprs_[1]->evaluate().as_iri();
		if (datatype.null()) {
			return Literal{};
		}

		return Literal::make_typed(lex_form.lexical_form(), datatype);
	}

	IMPL_CLONE(StrDt);

	/* Str Expression */
	node_wrapper::NodeWrapper Str::evaluate() const {
		auto expr_result = op_exprs_[0]->evaluate();

		if (auto expr_result_iri = expr_result.as_iri(); not expr_result_iri.null()) {
			return Literal::make_simple(expr_result_iri.identifier());
		}

		if (auto expr_result_literal = expr_result.as_literal(); not expr_result_literal.null()) {
			return Literal::make_simple(expr_result_literal.lexical_form().view());
		}

		return Literal{};
	}

	IMPL_CLONE(Str);

	/* Contains Expression */
	node_wrapper::NodeWrapper Contains::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_contains(op_exprs_[1]->evaluate().as_literal());
	}

	IMPL_CLONE(Contains);

	/* Concat Expression */
	node_wrapper::NodeWrapper Concat::evaluate() const {
		assert(not op_exprs_.empty());

		auto string_like = [](auto x) noexcept {
			return x.template datatype_eq<datatypes::xsd::String>()
			        or x.template datatype_eq<datatypes::rdf::LangString>();
		};

		auto acc = op_exprs_[0]->evaluate().as_literal();
		if (not string_like(acc)) {
			return Literal{};
		}

		for (size_t ix = 1; ix < op_exprs_.size(); ++ix) {
			auto res = op_exprs_[ix]->evaluate().as_literal();
			if (not string_like(res)) {
				return Literal{};
			}

			acc = acc.concat(res);
		}

		return acc;
	}

	IMPL_CLONE(Concat);

	/* StrStarts Expression */
	node_wrapper::NodeWrapper StrStarts::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_str_starts_with(op_exprs_[1]->evaluate().as_literal());
	}

	IMPL_CLONE(StrStarts);

	/* StrBefore Expression */
	node_wrapper::NodeWrapper StrBefore::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().substr_before(op_exprs_[1]->evaluate().as_literal());
	}

	IMPL_CLONE(StrBefore);

	/* StrAfter Expression */
	node_wrapper::NodeWrapper StrAfter::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().substr_after(op_exprs_[1]->evaluate().as_literal());
	}

	IMPL_CLONE(StrAfter);

	/* StrEnds Expression */
	node_wrapper::NodeWrapper StrEnds::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_str_ends_with(op_exprs_[1]->evaluate().as_literal());
	}

	IMPL_CLONE(StrEnds);

	/* LangMatches Expression */
	node_wrapper::NodeWrapper LangMatches::evaluate() const {
		return lang_matches(op_exprs_[0]->evaluate().as_literal(),
							op_exprs_[1]->evaluate().as_literal());
	}

	IMPL_CLONE(LangMatches);

	/* Lang Expression */
	node_wrapper::NodeWrapper Lang::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_language_tag();
	}

	IMPL_CLONE(Lang);

	/* StrLang Expression */
	node_wrapper::NodeWrapper StrLang::evaluate() const {
		auto lex_form = op_exprs_[0]->evaluate().as_literal();
		auto lang_tag = op_exprs_[1]->evaluate().as_literal();
		if (not lex_form.datatype_eq<datatypes::xsd::String>() or not lang_tag.datatype_eq<datatypes::xsd::String>())
			return Literal{};

		return Literal::make_lang_tagged(lex_form.lexical_form(), lang_tag.lexical_form());
	}

	IMPL_CLONE(StrLang);

	/* StrLen Expression */
	node_wrapper::NodeWrapper StrLen::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_strlen();
	}

	IMPL_CLONE(StrLen);

	/* UCase Expression */
	node_wrapper::NodeWrapper UCase::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().uppercase();
	}

	IMPL_CLONE(UCase);

	/* LCase Expression */
	node_wrapper::NodeWrapper LCase::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().lowercase();
	}

	IMPL_CLONE(LCase);

	/* EncodeForURI Expression */
	node_wrapper::NodeWrapper EncodeForURI::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().encode_for_uri();
	}

	IMPL_CLONE(EncodeForURI);

	/* If Expression */
	node_wrapper::NodeWrapper If::evaluate() const {
		auto condition = op_exprs_[0]->evaluate();
		if (condition.null()) {
			return {};
		}

		return condition ? op_exprs_[1]->evaluate()
						 : op_exprs_[2]->evaluate();
	}

	IMPL_CLONE(If);

	/* SubStr Expression */
	node_wrapper::NodeWrapper SubStr2::evaluate() const {
		auto str = op_exprs_[0]->evaluate().as_literal();
		if (str.null()) {
			return Literal{};
		}

		auto start = op_exprs_[1]->evaluate().as_literal();
		return str.substr(start);
	}

	IMPL_CLONE(SubStr2);

	node_wrapper::NodeWrapper SubStr3::evaluate() const {
		auto str = op_exprs_[0]->evaluate().as_literal();
		if (str.null()) {
			return Literal{};
		}

		auto start = op_exprs_[1]->evaluate().as_literal();
		auto len = op_exprs_[2]->evaluate().as_literal();
		return str.substr(start, len);
	}

	IMPL_CLONE(SubStr3);

	/* Coalesce Expression */
	node_wrapper::NodeWrapper Coalesce::evaluate() const {
		for (auto const &expr : op_exprs_) {
			if (auto expr_res = expr->evaluate(); not expr_res.null()) {
				return expr_res;
			}
		}

		return {};
	}

	IMPL_CLONE(Coalesce);

	/* Regex Expression */
	node_wrapper::NodeWrapper Regex2::evaluate() const {
		auto str = op_exprs_[0]->evaluate().as_literal();
		auto pattern = op_exprs_[1]->evaluate().as_literal();

		return str.as_regex_matches(pattern);
	}

	IMPL_CLONE(Regex2);

	node_wrapper::NodeWrapper Regex3::evaluate() const {
		auto str = op_exprs_[0]->evaluate().as_literal();
		auto pattern = op_exprs_[1]->evaluate().as_literal();
		auto flags = op_exprs_[2]->evaluate().as_literal();

		return str.as_regex_matches(pattern, flags);
	}

	IMPL_CLONE(Regex3);

	/* Replace Expression */
	node_wrapper::NodeWrapper Replace3::evaluate() const {
		auto str = op_exprs_[0]->evaluate().as_literal();
		auto pattern = op_exprs_[1]->evaluate().as_literal();
		auto replacement = op_exprs_[2]->evaluate().as_literal();

		return str.regex_replace(pattern, replacement);
	}

	IMPL_CLONE(Replace3);

	node_wrapper::NodeWrapper Replace4::evaluate() const {
		auto str = op_exprs_[0]->evaluate().as_literal();
		auto pattern = op_exprs_[1]->evaluate().as_literal();
		auto replacement = op_exprs_[2]->evaluate().as_literal();
		auto flags = op_exprs_[3]->evaluate().as_literal();

		return str.regex_replace(pattern, replacement, flags);
	}

	IMPL_CLONE(Replace4);

	/* Bound Expression */
	node_wrapper::NodeWrapper Bound::evaluate() const {
		return Literal::make_boolean(not op_exprs_[0]->evaluate().null());
	}

	IMPL_CLONE(Bound);

	/* UUID Expression */
	node_wrapper::NodeWrapper UUID::evaluate() const {
		return rdf4cpp::rdf::IRI::make_uuid();
	}

	IMPL_UNIT_CLONE(UUID);

	/* StrUUID Expression */
	node_wrapper::NodeWrapper StrUUID::evaluate() const {
		return rdf4cpp::rdf::Literal::make_string_uuid();
	}

	IMPL_UNIT_CLONE(StrUUID);

	/* Rand Expression */
	node_wrapper::NodeWrapper Rand::evaluate() const {
		return rdf4cpp::rdf::Literal::generate_random_double();
	}

	IMPL_UNIT_CLONE(Rand);

	/* Now Expression */
	node_wrapper::NodeWrapper Now::evaluate() const { assert(false); return {};}

	node_wrapper::NodeWrapper Now::evaluate(detail::EvaluationContext &eval_ctx) const {
		auto start_time_ms = std::chrono::floor<std::chrono::milliseconds>(eval_ctx.get_start_time());
		util::Timezone tz{};
		util::TimePoint tp = tz.to_local(start_time_ms);
		util::OptionalTimezone opt = std::nullopt;

		return Literal::make_typed_from_value<datatypes::xsd::DateTime>(std::make_pair(tp, opt));
	}

	IMPL_UNIT_CLONE(Now);

	/* Year Expression */
	node_wrapper::NodeWrapper Year::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_year();
	}

	IMPL_CLONE(Year);

	/* Month Expression */
	node_wrapper::NodeWrapper Month::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_month();
	}

	IMPL_CLONE(Month);

	/* Day Expression */
	node_wrapper::NodeWrapper Day::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_day();
	}

	IMPL_CLONE(Day);

	/* Hours Expression */
	node_wrapper::NodeWrapper Hours::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_hours();
	}

	IMPL_CLONE(Hours);

	/* Minutes Expression */
	node_wrapper::NodeWrapper Minutes::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_minutes();
	}

	IMPL_CLONE(Minutes);

	/* Seconds Expression */
	node_wrapper::NodeWrapper Seconds::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_seconds();
	}

	IMPL_CLONE(Seconds);

	/* Timezone Expression */
	node_wrapper::NodeWrapper Timezone::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_timezone();
	}

	IMPL_CLONE(Timezone);

	/* TZ Expression */
	node_wrapper::NodeWrapper TZ::evaluate() const {
		return op_exprs_[0]->evaluate().as_literal().as_tz();
	}

	IMPL_CLONE(TZ);

}// namespace dice::sparql::expressions
