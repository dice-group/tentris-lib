#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <dice/tentris/hypertrie-template-instantiation.hpp>
#include <dice/query.hpp>

#include "Expressions.hpp"

#include <algorithm>

namespace dice::query::tests {

	using htt_t = hypertrie::default_bool_Hypertrie_trait;
	using allocator_type = std::allocator<std::byte>;

	bool evaluate(Query<htt_t, allocator_type> &query,
				  hypertrie::const_Hypertrie<htt_t, allocator_type> &tensor,
				  OperandDependencyGraph<htt_t, allocator_type> &&odg,
				  std::vector<Key<size_t, htt_t>> &expected_results) {
		std::vector<Key<size_t, htt_t>> actual_results{};
		query.set_operand_dependency_graph(std::move(odg));
		EvaluationContext<htt_t, allocator_type> ctx{tensor};
		for (const auto &res : Evaluation::evaluate<htt_t, allocator_type>(query, ctx)) {
			std::cout << to_string(res) << std::endl;
			for (size_t i = 0; i < res.value(); i++) {
				actual_results.emplace_back(res.key());
			}
		}
		std::sort(expected_results.begin(), expected_results.end());
		std::sort(actual_results.begin(), actual_results.end());
		std::cout << " ----- " << std::endl;
		return (actual_results == expected_results);
	}

	TEST_CASE("Join") {
		Query<htt_t, allocator_type> query{std::make_shared<hypertrie::HypertrieContext<htt_t, allocator_type>>(allocator_type{})};
		hypertrie::Hypertrie<htt_t, allocator_type> tensor{3};
		OperandDependencyGraph<htt_t, allocator_type> odg{};
		tensor.set({1, 100, 2}, true);
		tensor.set({10, 100, 5}, true);
		tensor.set({2, 200, 5}, true);
		tensor.set({2, 200, 6}, true);
		hypertrie::Hypertrie<htt_t, allocator_type> true_scalar{0};
		detail::SliceKey<htt_t> s_key1{std::nullopt, 100, std::nullopt};
		detail::SliceKey<htt_t> s_key2{std::nullopt, 200, std::nullopt};
		true_scalar.set({}, true);
		odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
		odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
		odg.add_dependencies(0, 1, true);
		track_and_bindings(query, 3);
		std::vector<Key<size_t, htt_t>> expected_results;
		SUBCASE("Simple Join") {
			expected_results = {
					{1, 2, 5},
					{1, 2, 6}};
		}
		SUBCASE("Join and Projected SquareExpression") {
			auto expr = ExpressionWrapper<htt_t, allocator_type>{std::make_unique<SquareExpression>(gen_binding(2))};
			query.add_binding(std::move(expr));
			expected_results = {
					{1, 2, 5, 25},
					{1, 2, 6, 36}};
		}
		SUBCASE("Join and Filter") {
			SUBCASE("True Filter") {
				auto filter = ExpressionWrapper<htt_t, allocator_type>{std::make_unique<TrueExpression>()};
				odg.add_filter({'a'}, FilterExpression<htt_t, allocator_type>(std::move(filter)));
				expected_results = { {1, 2, 5}, {1, 2, 6} };
			}
			SUBCASE("False Filter") {
				auto filter = ExpressionWrapper<htt_t, allocator_type>{std::make_unique<FalseExpression>()};
				odg.add_filter({'a'}, FilterExpression<htt_t, allocator_type>(std::move(filter)));
				expected_results = {};
			}
			odg.add_dependencies(0, 2, true);
			odg.add_dependencies(1, 2, true);
		}
		CHECK(evaluate(query, tensor, std::move(odg), expected_results));
	}

	TEST_CASE("Left Join") {
		hypertrie::Hypertrie<htt_t, allocator_type> ht{3};
		ht.set({1, 10, 20}, true);
		ht.set({2, 10, 20}, true);
		ht.set({3, 10, 20}, true);
		ht.set({4, 10, 20}, true);
		ht.set({5, 10, 20}, true);
		ht.set({6, 10, 20}, true);
		ht.set({7, 10, 20}, true);
		ht.set({8, 10, 20}, true);
		ht.set({1, 11, 3}, true);
		ht.set({1, 11, 6}, true);
		ht.set({2, 11, 4}, true);
		ht.set({1, 12, 8}, true);
		ht.set({4, 12, 6}, true);
		ht.set({3, 12, 5}, true);
		ht.set({4, 13, 25}, true);
		ht.set({3, 13, 25}, true);
		ht.set({5, 14, 30}, true);
		ht.set({1, 14, 35}, true);
		ht.set({8, 14, 30}, true);
		ht.set({1, 15, 40}, true);
		ht.set({6, 15, 55}, true);
		ht.set({2, 16, 45}, true);
		ht.set({3, 16, 50}, true);
		detail::SliceKey<htt_t> s_key1{std::nullopt, 10, 20};
		detail::SliceKey<htt_t> s_key2{std::nullopt, 11, std::nullopt};
		detail::SliceKey<htt_t> s_key3{std::nullopt, 12, std::nullopt};
		detail::SliceKey<htt_t> s_key4{std::nullopt, 13, std::nullopt};
		detail::SliceKey<htt_t> s_key5{std::nullopt, 14, std::nullopt};
		detail::SliceKey<htt_t> s_key6{std::nullopt, 15, std::nullopt};
		detail::SliceKey<htt_t> s_key7{std::nullopt, 16, std::nullopt};
		auto default_key_part = std::numeric_limits<htt_t::key_part_type>::min();
		Query<htt_t, allocator_type> query{std::make_shared<hypertrie::HypertrieContext<htt_t, allocator_type>>(allocator_type{})};
		OperandDependencyGraph<htt_t, allocator_type> odg{};
		std::vector<Key<size_t, htt_t>> expected_results;
		SUBCASE("Single Right Operand") {
			SUBCASE("LJ(a, ab), Project: a") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_dependencies(0, 1, false);
				track_and_bindings(query, 1);
				SUBCASE("Not Distinct") {
					expected_results = { {1}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8} };
				}
				SUBCASE("Distinct") {
					query.set_distinct();
					expected_results = { {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8} };
				}
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a, ab), Project: ab") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_dependencies(0, 1, false);
				track_and_bindings(query, 2);
				expected_results = {
						{1, 3},
						{1, 6},
						{2, 4},
						{3, default_key_part},
						{4, default_key_part},
						{5, default_key_part},
						{6, default_key_part},
						{7, default_key_part},
						{8, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(ab, bc), Project: abc") {
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, false);
				track_and_bindings(query, 3);
				expected_results = {
						{1, 3, 5},
						{1, 6, default_key_part},
						{2, 4, 6}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
		}
		SUBCASE("Multiple Right Operands") {
			SUBCASE("LJ(LJ(a, ab), ac)), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_connection(1, 2);
				odg.add_connection(2, 1);
				track_and_bindings(query, 3);
				expected_results = {
						{1, 3, 8},
						{1, 6, 8},
						{2, 4, default_key_part},
						{3, default_key_part, 5},
						{4, default_key_part, 6},
						{5, default_key_part, default_key_part},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(LJ(LJ(a, ab), ac) ad), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(0, 3, false);
				odg.add_connection(1, 2);
				odg.add_connection(2, 1);
				odg.add_connection(1, 3);
				odg.add_connection(3, 1);
				odg.add_connection(2, 3);
				odg.add_connection(3, 2);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 8, 35},
						{1, 6, 8, 35},
						{2, 4, default_key_part, default_key_part},
						{3, default_key_part, 5, default_key_part},
						{4, default_key_part, 6, default_key_part},
						{5, default_key_part, default_key_part, 30},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, 30}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(LJ(ab, ac), bd)), Project: abc") {
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'b', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_connection(1, 2);
				odg.add_connection(2, 1);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 8, 25},
						{1, 6, 8, default_key_part},
						{2, 4, default_key_part, 25}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
		}
		SUBCASE("Nested Operations") {
			SUBCASE("LJ(a, LJ(ab, bc)), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(1, 2, false);
				track_and_bindings(query, 3);
				expected_results = {
						{1, 3, 5},
						{1, 6, default_key_part},
						{2, 4, 6},
						{3, default_key_part, default_key_part},
						{4, default_key_part, default_key_part},
						{5, default_key_part, default_key_part},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a, LJ(ab, LJ(bc, cd)), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'c', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(1, 2, false);
				odg.add_dependencies(2, 3, false);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 5, 30},
						{1, 6, default_key_part, default_key_part},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a, LJ(ab, ac)), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(1, 2, false);
				track_and_bindings(query, 3);
				expected_results = {
						{1, 3, 8},
						{1, 6, 8},
						{2, 4, default_key_part},
						{3, default_key_part, default_key_part},
						{4, default_key_part, default_key_part},
						{5, default_key_part, default_key_part},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a, LJ(ab, LJ(ac, ad)), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key6));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key7));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(1, 2, false);
				odg.add_dependencies(2, 3, false);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 40, default_key_part},
						{1, 6, 40, default_key_part},
						{2, 4, default_key_part, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
		}
	}

	TEST_CASE("Join Combinations") {
		hypertrie::Hypertrie<htt_t, allocator_type> ht{3};
		ht.set({1, 10, 20}, true);
		ht.set({2, 10, 20}, true);
		ht.set({3, 10, 20}, true);
		ht.set({4, 10, 20}, true);
		ht.set({5, 10, 20}, true);
		ht.set({6, 10, 20}, true);
		ht.set({7, 10, 20}, true);
		ht.set({8, 10, 20}, true);
		ht.set({1, 11, 3}, true);
		ht.set({1, 11, 6}, true);
		ht.set({2, 11, 4}, true);
		ht.set({5, 11, 7}, true);
		ht.set({1, 12, 8}, true);
		ht.set({4, 12, 6}, true);
		ht.set({3, 12, 5}, true);
		ht.set({4, 13, 25}, true);
		ht.set({6, 13, 25}, true);
		ht.set({5, 14, 30}, true);
		ht.set({1, 14, 35}, true);
		ht.set({8, 14, 30}, true);
		ht.set({8, 15, 35}, true);
		ht.set({7, 15, 30}, true);
		detail::SliceKey<htt_t> s_key1{std::nullopt, 10, 20};
		detail::SliceKey<htt_t> s_key2{std::nullopt, 11, std::nullopt};
		detail::SliceKey<htt_t> s_key3{std::nullopt, 12, std::nullopt};
		detail::SliceKey<htt_t> s_key4{std::nullopt, 13, std::nullopt};
		detail::SliceKey<htt_t> s_key5{std::nullopt, 14, std::nullopt};
		detail::SliceKey<htt_t> s_key6{std::nullopt, 16, std::nullopt};
		detail::SliceKey<htt_t> s_key7{std::nullopt, 15, std::nullopt};
		auto default_key_part = std::numeric_limits<htt_t::key_part_type>::min();
		Query<htt_t, allocator_type> query{std::make_shared<hypertrie::HypertrieContext<htt_t, allocator_type>>(allocator_type{})};
		OperandDependencyGraph<htt_t, allocator_type> odg{};
		std::vector<Key<size_t, htt_t>> expected_results;
		SUBCASE("Join and Left Join") {
			SUBCASE("LJ(J(a,ab), bc), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(1, 2, false);
				odg.add_dependencies(0, 2, false);
				track_and_bindings(query, 3);
				expected_results = {
						{1, 3, 5},
						{1, 6, default_key_part},
						{2, 4, 6},
						{5, 7, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(J(a,ab), ac), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, false);
				track_and_bindings(query, 3);
				expected_results = {
						{1, 3, 8},
						{1, 6, 8},
						{2, 4, default_key_part},
						{5, 7, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a, J(ab,bc)), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				track_and_bindings(query, 3);
				expected_results = {
						{1, 3, 5},
						{2, 4, 6},
						{3, default_key_part, default_key_part},
						{4, default_key_part, default_key_part},
						{5, default_key_part, default_key_part},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a, J(ab,bc)), Project: a") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				track_and_bindings(query, 1);
				expected_results = {
						{1},
						{2},
						{3},
						{4},
						{5},
						{6},
						{7},
						{8}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(J(a,J(ab, bc)), cd), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'c', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(0, 2, true);
				odg.add_dependencies(0, 3, false);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(1, 3, false);
				odg.add_dependencies(2, 3, false);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 5, 30},
						{2, 4, 6, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a,LJ(J(ab, bc)), cd), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'c', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(0, 3, false);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(1, 3, false);
				odg.add_dependencies(2, 3, false);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 5, 30},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a,J(ab, ac)), Project: abc") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				track_and_bindings(query, 3);
				expected_results = {
						{1, 3, 35},
						{1, 6, 35},
						{2, default_key_part, default_key_part},
						{3, default_key_part, default_key_part},
						{4, default_key_part, default_key_part},
						{5, 7, 30},
						{6, default_key_part, default_key_part},
						{7, default_key_part, default_key_part},
						{8, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a,J(ab, ac)), Project: a") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				track_and_bindings(query, 1);
				expected_results = {
						{1},
						{1},
						{2},
						{3},
						{4},
						{5},
						{6},
						{7},
						{8}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a,J(ab, ac)), Project: a (DISTINCT)") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				track_and_bindings(query, 1);
				query.set_distinct();
				std::vector<Key<size_t, htt_t>> expected_results_dist = {
						{1},
						{2},
						{3},
						{4},
						{5},
						{6},
						{7},
						{8}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results_dist));
			}
			SUBCASE("J(LJ(a,J(ab, bc)), ad), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(3, 1, false);
				odg.add_dependencies(3, 2, false);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(0, 3, true);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 5, 35},
						{5, default_key_part, default_key_part, 30},
						{8, default_key_part, default_key_part, 30}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(LJ(a,J(ab, bc)), ad), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(0, 3, false);
				odg.add_connection(2, 3);
				odg.add_connection(3, 2);
				odg.add_connection(1, 3);
				odg.add_connection(3, 1);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 5, 35},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, 30},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, 30}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a,LJ(J(ab, bc), ad)), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(1, 3, false);
				odg.add_dependencies(2, 3, false);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 5, 35},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a,LJ(J(ab,J(bc,ce)),ad)), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'c', 'e'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(2, 3, true);
				odg.add_dependencies(1, 3, true);
				odg.add_dependencies(1, 4, false);
				odg.add_dependencies(2, 4, false);
				odg.add_dependencies(3, 4, false);
				track_and_bindings(query, 4);
				expected_results = {
						{1, default_key_part, default_key_part, default_key_part},
						{2, 4, 6, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(J(a,J(ab,bc)),ad), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(0, 2, true);
				odg.add_dependencies(0, 3, false);
				odg.add_dependencies(1, 3, false);
				odg.add_dependencies(2, 3, false);
				track_and_bindings(query, 4);
				expected_results = {
						{1, 3, 5, 35},
						{2, 4, 6, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(LJ(J(a,J(ab,bc)),ad),be), Project: abcde") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_operand({'b', 'e'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(1, 3, false);
				odg.add_dependencies(2, 3, false);
				odg.add_dependencies(2, 4, false);
				odg.add_connection(3, 4);
				odg.add_connection(4, 3);
				track_and_bindings(query, 5);
				expected_results = {
						{1, 3, 5, 35, default_key_part},
						{2, 4, 6, default_key_part, 25}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("J(LJ(J(a,J(ab,bc)),ad),be), Project: abcde") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_operand({'b', 'e'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(0, 2, true);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(1, 3, false);
				odg.add_dependencies(2, 3, false);
				odg.add_dependencies(2, 4, true);
				odg.add_dependencies(4, 3, false);
				odg.add_dependencies(0, 4, true);
				odg.add_dependencies(1, 4, true);
				odg.add_dependencies(2, 4, true);
				track_and_bindings(query, 5);
				expected_results = {
						{2, 4, 6, default_key_part, 25}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(J(a,J(ab,J(bc,ce))),bd), Project: abcde") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'c', 'e'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_operand({'b', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(0, 2, true);
				odg.add_dependencies(0, 3, true);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(1, 3, true);
				odg.add_dependencies(2, 3, true);
				odg.add_dependencies(0, 4, false);
				odg.add_dependencies(1, 4, false);
				odg.add_dependencies(2, 4, false);
				odg.add_dependencies(3, 4, false);
				track_and_bindings(query, 5);
				expected_results = {
						{1, 3, 5, default_key_part, 30}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
		}
		SUBCASE("Left Join and Cartesian") {
			SUBCASE("LJ(a, J(bc,cd)), Project: abcd") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_operand({'c', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				track_and_bindings(query, 4);
				expected_results = {
						{1, default_key_part, default_key_part, default_key_part},
						{2, default_key_part, default_key_part, default_key_part},
						{3, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(LJ(J(ab,bh),cd),J(eg,ef)), Project: abcdef") {
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'h'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_operand({'c', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'e', 'g'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_operand({'e', 'f'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				// joins
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(3, 4, true);
				// cartesian optional
				odg.add_connection(0, 2);
				odg.add_connection(1, 2);
				odg.add_connection(2, 0);
				odg.add_connection(2, 1);
				odg.add_connection(2, 3);
				odg.add_connection(2, 4);
				odg.add_connection(3, 2);
				odg.add_connection(4, 2);
				odg.add_connection(0, 3);
				odg.add_connection(3, 0);
				odg.add_connection(0, 4);
				odg.add_connection(4, 0);
				odg.add_connection(1, 3);
				odg.add_connection(3, 1);
				odg.add_connection(1, 4);
				odg.add_connection(4, 1);
				track_and_bindings(query, 6);
				std::vector<Key<size_t, htt_t>> expected_results1 = {
						{default_key_part, default_key_part}};
				std::vector<Key<size_t, htt_t>> expected_results2 = {
						{1, 8},
						{4, 6},
						{3, 5}};
				std::vector<Key<size_t, htt_t>> expected_results3 = {
						{default_key_part, default_key_part}};
				for (auto &res1 : expected_results1) {
					for (auto &res2 : expected_results2) {
						for (auto &res3 : expected_results3) {
							auto temp_res = res1;
							temp_res.as_inner().insert(temp_res.end(), res2.begin(), res2.end());
							temp_res.as_inner().insert(temp_res.end(), res3.begin(), res3.end());
							expected_results.push_back(temp_res);
						}
					}
				}
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(a,J(ab,cd)), Project:ab") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'d', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key6));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				track_and_bindings(query, 2);
				expected_results = {
						{1, default_key_part},
						{2, default_key_part},
						{3, default_key_part},
						{4, default_key_part},
						{5, default_key_part},
						{6, default_key_part},
						{7, default_key_part},
						{8, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("J(LJ(a,(ab),LJ(c,cd)), Project:abcd") {

				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'c', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, true);
				odg.add_dependencies(0, 3, false);
				odg.add_dependencies(2, 3, false);
				odg.add_dependencies(2, 1, false);
				odg.add_connection(1, 3);
				odg.add_connection(3, 1);
				track_and_bindings(query, 4);
				std::vector<Key<size_t, htt_t>> expected_results1 = {
						{1, 3},
						{1, 6},
						{2, 4},
						{3, default_key_part},
						{4, default_key_part},
						{5, 7},
						{6, default_key_part},
						{7, default_key_part},
						{8, default_key_part}};
				std::vector<Key<size_t, htt_t>> expected_results2 = {
						{1, 8},
						{2, default_key_part},
						{3, 5},
						{4, 6},
						{5, default_key_part},
						{6, default_key_part},
						{7, default_key_part},
						{8, default_key_part}};
				for (auto &res1 : expected_results1) {
					for (auto &res2 : expected_results2) {
						auto temp_res = res1;
						temp_res.as_inner().insert(temp_res.end(), res2.begin(), res2.end());
						expected_results.push_back(temp_res);
					}
				}
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(LJ(J(ab,bh),cd),J(eg,ef)), Project:abcdef") {
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'b', 'h'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_operand({'c', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'e', 'g'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_operand({'e', 'f'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, true);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, false);
				odg.add_dependencies(0, 3, false);
				odg.add_dependencies(1, 3, false);
				odg.add_dependencies(3, 4, true);
				odg.add_connection(2, 3);
				odg.add_connection(3, 2);
				track_and_bindings(query, 6);
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
			SUBCASE("LJ(LJ(a,LJ(J(ab,bc),ad)),LJ(J(ae,ef),ag)), Project:abdeg") {
				odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
				odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
				odg.add_operand({'b', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_operand({'a', 'e'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
				odg.add_operand({'e', 'f'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
				odg.add_operand({'a', 'g'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
				odg.add_dependencies(0, 1, false);
				odg.add_dependencies(0, 2, false);
				odg.add_dependencies(1, 2, true);
				odg.add_dependencies(1, 3, false);
				odg.add_dependencies(2, 3, false);

				odg.add_dependencies(0, 4, false);
				odg.add_dependencies(0, 5, false);
				odg.add_dependencies(4, 5, true);
				odg.add_dependencies(4, 6, false);
				odg.add_dependencies(5, 6, false);

				odg.add_connection(1, 4);
				odg.add_connection(4, 1);
				odg.add_connection(2, 4);
				odg.add_connection(4, 2);
				odg.add_connection(1, 5);
				odg.add_connection(5, 1);
				odg.add_connection(2, 5);
				odg.add_connection(5, 2);
				query.track_variable('a');
				query.add_binding(ExpressionWrapper<htt_t, allocator_type>{gen_binding(0)});
				query.track_variable('b');
				query.add_binding(ExpressionWrapper<htt_t, allocator_type>{gen_binding(1)});
				query.track_variable('d');
				query.add_binding(ExpressionWrapper<htt_t, allocator_type>{gen_binding(2)});
				query.track_variable('e');
				query.add_binding(ExpressionWrapper<htt_t, allocator_type>{gen_binding(3)});
				query.track_variable('g');
				query.add_binding(ExpressionWrapper<htt_t, allocator_type>{gen_binding(4)});
				expected_results = {
						{1, default_key_part, default_key_part, 6, 35},
						{2, default_key_part, default_key_part, 4, default_key_part},
						{3, 5, default_key_part, default_key_part, default_key_part},
						{4, default_key_part, default_key_part, default_key_part, default_key_part},
						{5, default_key_part, default_key_part, default_key_part, default_key_part},
						{6, default_key_part, default_key_part, default_key_part, default_key_part},
						{7, default_key_part, default_key_part, default_key_part, default_key_part},
						{8, default_key_part, default_key_part, default_key_part, default_key_part}};
				CHECK(evaluate(query, ht, std::move(odg), expected_results));
			}
		}
	}

	TEST_CASE("Union") {
		auto default_key_part = std::numeric_limits<htt_t::key_part_type>::min();
		hypertrie::Hypertrie<htt_t, allocator_type> ht{3};
		ht.set({1, 10, 20}, true);
		ht.set({2, 10, 20}, true);
		ht.set({3, 10, 20}, true);
		ht.set({4, 10, 20}, true);
		ht.set({5, 10, 20}, true);
		ht.set({6, 10, 20}, true);
		ht.set({7, 10, 20}, true);
		ht.set({8, 10, 20}, true);
		ht.set({1, 11, 3}, true);
		ht.set({1, 11, 6}, true);
		ht.set({2, 11, 4}, true);
		ht.set({1, 12, 8}, true);
		ht.set({4, 12, 6}, true);
		ht.set({3, 12, 5}, true);
		ht.set({5, 13, 50}, true);
		ht.set({6, 13, 50}, true);
		ht.set({2, 13, 60}, true);
		ht.set({4, 13, 60}, true);
		detail::SliceKey<htt_t> s_key1{std::nullopt, 10, 20};
		detail::SliceKey<htt_t> s_key2{std::nullopt, 11, std::nullopt};
		detail::SliceKey<htt_t> s_key3{std::nullopt, 12, std::nullopt};
		detail::SliceKey<htt_t> s_key4{std::nullopt, 13, 50};
		detail::SliceKey<htt_t> s_key5{std::nullopt, 13, 60};
		detail::SliceKey<htt_t> s_key6{std::nullopt, 13, std::nullopt};
		Query<htt_t, allocator_type> query{std::make_shared<hypertrie::HypertrieContext<htt_t, allocator_type>>(allocator_type{})};
		OperandDependencyGraph<htt_t, allocator_type> odg{};
		std::vector<Key<size_t, htt_t>> expected_results;
		SUBCASE("U(a,b), Project: a)") {
			// operand for 'a' is empty, still need to produce a mapping with 'a' being unbound
			detail::SliceKey<htt_t> s_key_empty{std::nullopt, 16, 20};
			detail::SliceKey<htt_t> s_key_nonempty{1, 11, std::nullopt};
			odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key_empty));
			odg.add_operand({'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key_nonempty));
			track_and_bindings(query, 1);
			expected_results = {
					{default_key_part},
					{default_key_part}};
			CHECK(evaluate(query, ht, std::move(odg), expected_results));
		}
		SUBCASE("J(a,U(ab,ac)), Project: abc") {
			odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
			odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
			odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key1));
			odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
			odg.add_dependencies(0, 1, true);
			odg.add_dependencies(2, 3, true);
			track_and_bindings(query, 3);
			expected_results = {
					{1, 3, default_key_part},
					{1, 6, default_key_part},
					{2, 4, default_key_part},
					{1, default_key_part, 8},
					{4, default_key_part, 6},
					{3, default_key_part, 5}};
			CHECK(evaluate(query, ht, std::move(odg), expected_results));
		}
		SUBCASE("J(ab,U(a,a)), Project: ab") {
			odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
			odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key4));
			odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
			odg.add_operand({'a'}, std::make_shared<detail::SliceKey<htt_t>>(s_key5));
			odg.add_dependencies(0, 1, true);
			odg.add_dependencies(2, 3, true);
			track_and_bindings(query, 2);
			expected_results = {
					{2, 4}};
			CHECK(evaluate(query, ht, std::move(odg), expected_results));
		}
		SUBCASE("U(ab,ac), Project: abc") {
			odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
			odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
			track_and_bindings(query, 3);
			expected_results = {
					{1, 3, default_key_part},
					{1, 6, default_key_part},
					{2, 4, default_key_part},
					{1, default_key_part, 8},
					{4, default_key_part, 6},
					{3, default_key_part, 5}};
			CHECK(evaluate(query, ht, std::move(odg), expected_results));
		}
		SUBCASE("LJ(U(ab,ac),ad), Project: abcd") {
			odg.add_operand({'a', 'b'}, std::make_shared<detail::SliceKey<htt_t>>(s_key2));
			odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key6));
			odg.add_operand({'a', 'c'}, std::make_shared<detail::SliceKey<htt_t>>(s_key3));
			odg.add_operand({'a', 'd'}, std::make_shared<detail::SliceKey<htt_t>>(s_key6));
			odg.add_dependencies(0, 1, false);
			odg.add_dependencies(2, 3, false);
			track_and_bindings(query, 4);
			expected_results = {
					{1, 3, default_key_part, default_key_part},
					{1, 6, default_key_part, default_key_part},
					{2, 4, default_key_part, 60},
					{1, default_key_part, 8, default_key_part},
					{4, default_key_part, 6, 60},
					{3, default_key_part, 5, default_key_part}};
			CHECK(evaluate(query, ht, std::move(odg), expected_results));
		}
	}

	TEST_CASE("simulate VALUES") {
		Query<htt_t, allocator_type> query{std::make_shared<hypertrie::HypertrieContext<htt_t, allocator_type>>(allocator_type{})};

		size_t id;
		{
			hypertrie::Hypertrie<htt_t, allocator_type> hyp{2, query.tensor_context().get()};
			hyp.set({1, 2}, true);

			id = query.add_inline_data(std::move(hyp));
		}

		auto const &hyp = query.inline_data(id);

		CHECK_EQ(hyp.size(), 1);
		CHECK_EQ(hyp[hypertrie::Key<htt_t>{1, 2}], true);
	}

}// namespace dice::query::tests
