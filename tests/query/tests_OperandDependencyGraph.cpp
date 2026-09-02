#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "Expressions.hpp"
#include "dice/query/OperandDependencyGraph.hpp"

namespace dice::query::tests {

	using pos_type = hypertrie::internal::pos_type;
	using htt_t = hypertrie::default_bool_Hypertrie_trait;
	using allocator_type = std::allocator<std::byte>;
	using OperandDependencyGraph = query::OperandDependencyGraph<htt_t, allocator_type>;
	using SliceKey = hypertrie::SliceKey<htt_t>;

	TEST_CASE("Graph Merging") {
		OperandDependencyGraph first{};
		first.add_operand({'a'}, std::make_shared<SliceKey>());
		OperandDependencyGraph second{};
		second.add_operand({'b'}, std::make_shared<SliceKey>());
		auto merged_graph = OperandDependencyGraph::merge_graphs(first, second);
		CHECK(merged_graph.size() == 2);
		CHECK(merged_graph.operand_var_ids(0) == std::vector<char>{'a'});
		CHECK(merged_graph.operand_var_ids(1) == std::vector<char>{'b'});
		CHECK(merged_graph.union_components().size() == 2);
		OperandDependencyGraph third{};
		third.add_operand({'c'}, std::make_shared<SliceKey>());
		merged_graph = OperandDependencyGraph::merge_graphs(merged_graph, third);
		CHECK(merged_graph.size() == 3);
		CHECK(merged_graph.operand_var_ids(0) == std::vector<char>{'a'});
		CHECK(merged_graph.operand_var_ids(1) == std::vector<char>{'b'});
		CHECK(merged_graph.operand_var_ids(2) == std::vector<char>{'c'});
		CHECK(merged_graph.union_components().size() == 3);
	}

	TEST_CASE("Label Removal") {
		query::OperandDependencyGraph<htt_t, allocator_type> odg{};
		odg.add_operand({'a'}, {});
		odg.add_operand({'a', 'b'}, {});
		odg.add_operand({'a', 'c'}, {});
		odg.add_operand({'a', 'd'}, {});
		odg.add_dependency(0, 1, 'a');
		odg.add_dependency(1, 0, 'a');
		odg.add_dependency(1, 2, 'a');
		odg.add_dependency(2, 1, 'a');
		odg.add_dependency(2, 3, 'a');
		odg.add_dependency(3, 2, 'a');
		auto &new_odg = odg.remove_var_id('a');
		CHECK_EQ(new_odg.size(), 3);
		CHECK_EQ(new_odg.operand_var_ids(0), std::vector<char>{'b'});
		CHECK_EQ(new_odg.operand_var_ids(1), std::vector<char>{'c'});
		CHECK_EQ(new_odg.operand_var_ids(2), std::vector<char>{'d'});
		auto copy_graph = odg;
	}

	TEST_CASE("Cartesian Components") {
		SUBCASE("Non Optional") {
			query::OperandDependencyGraph<htt_t, allocator_type> odg{};
			odg.add_operand({'b'}, {});
			odg.add_operand({'c'}, {});
			odg.add_operand({'d'}, {});
			odg.add_dependency(0, 1);
			odg.add_dependency(1, 0);
			odg.add_dependency(1, 2);
			odg.add_dependency(2, 1);
			odg.add_dependency(0, 2);
			odg.add_dependency(2, 0);
			auto &cc = odg.cartesian_components();
			CHECK_EQ(cc.size(), 3);
			CHECK_EQ(odg.optional_cartesian(), false);
		}
		SUBCASE("Optional") {
			query::OperandDependencyGraph<htt_t, allocator_type> odg{};
			odg.add_operand({'b'}, {});
			odg.add_operand({'c'}, {});
			odg.add_operand({'d'}, {});
			odg.add_connection(0, 1);
			odg.add_connection(1, 0);
			odg.add_connection(1, 2);
			odg.add_connection(2, 1);
			odg.add_connection(0, 2);
			odg.add_connection(2, 0);
			auto &cc = odg.cartesian_components();
			CHECK_EQ(cc.size(), 3);
			CHECK_EQ(odg.optional_cartesian(), true);
		}
	}

	TEST_CASE("Operands Original Positions") {
		SUBCASE("Cartesian Components") {
			query::OperandDependencyGraph<htt_t, allocator_type> odg{};
			odg.add_operand({'a'}, {});
			odg.add_operand({'b', 'c'}, {});
			odg.add_operand({'d', 'd'}, {});
			odg.add_dependency(0, 1);
			odg.add_dependency(1, 0);
			odg.add_dependency(0, 2);
			odg.add_dependency(2, 0);
			odg.add_dependency(1, 2, 'c');
			odg.add_dependency(2, 1, 'c');
			auto &cc1 = odg.cartesian_components();
			CHECK_EQ(cc1.size(), 2);
			CHECK_EQ(cc1[0].operands_original_positions(), std::vector<uint8_t>{0});
			CHECK_EQ(cc1[1].operands_original_positions(), std::vector<uint8_t>{1, 2});
			auto &cc2 = cc1[1].remove_var_id('c').cartesian_components();
			CHECK_EQ(cc2.size(), 2);
			CHECK_EQ(cc2[0].operands_original_positions(), std::vector<uint8_t>{0});
			CHECK_EQ(cc2[1].operands_original_positions(), std::vector<uint8_t>{1});
		}
		SUBCASE("Remove Vertices") {
			query::OperandDependencyGraph<htt_t, allocator_type> odg{};
			odg.add_operand({'a'}, {});
			odg.add_operand({'b', 'c'}, {});
			odg.add_operand({'c', 'd'}, {});
			odg.add_dependency(0, 1);
			odg.add_dependency(1, 0);
			odg.add_dependency(0, 2);
			odg.add_dependency(2, 0);
			odg.add_dependency(1, 2, 'c');
			odg.add_dependency(2, 1, 'c');
			auto &new_odg = odg.remove_vertices({0});
			CHECK_EQ(new_odg.operands_original_positions(), std::vector<uint8_t>{1, 2});
		}
		SUBCASE("Remove Vertices and Cartesian Components") {
			query::OperandDependencyGraph<htt_t, allocator_type> odg{};
			odg.add_operand({'a'}, {});
			odg.add_operand({'b', 'c'}, {});
			odg.add_operand({'d', 'e'}, {});
			odg.add_dependency(0, 1);
			odg.add_dependency(1, 0);
			odg.add_dependency(0, 2);
			odg.add_dependency(2, 0);
			odg.add_dependency(1, 2);
			odg.add_dependency(2, 1);
			auto &new_odg = odg.remove_vertices({0});
			CHECK_EQ(new_odg.operands_original_positions(), std::vector<uint8_t>{1, 2});
			auto &cc = new_odg.cartesian_components();
			CHECK_EQ(cc.size(), 2);
			CHECK_EQ(cc[0].operands_original_positions(), std::vector<uint8_t>{0});
			CHECK_EQ(cc[1].operands_original_positions(), std::vector<uint8_t>{1});
		}
	}

	TEST_CASE("Simple Graph") {
		query::OperandDependencyGraph<htt_t, allocator_type> odg{};
		odg.add_operand({'a', 'b'}, {});
		odg.add_operand({'b', 'c'}, {});
		odg.add_operand({'e'}, {});
		odg.add_operand({'c', 'd'}, {});
		odg.add_dependency(0, 1, 'b');
		odg.add_dependency(1, 0, 'b');
		odg.add_dependency(1, 3, 'c');
		odg.add_dependency(3, 1, 'c');
		odg.add_dependency(0, 2);
		odg.add_dependency(2, 0);
		odg.add_dependency(1, 2);
		odg.add_dependency(2, 1);
		odg.add_dependency(2, 3);
		odg.add_dependency(3, 2);
		SUBCASE("without unlabelled edges") {
			// basic testing
			CHECK_EQ(odg.size(), 4);
			CHECK_EQ(odg.operands_original_positions(), std::vector<pos_type>{0, 1, 2, 3});
			CHECK_EQ(odg.operand_var_ids(0), std::vector<char>{'a', 'b'});
			CHECK_EQ(odg.operand_var_ids(1), std::vector<char>{'b', 'c'});
			CHECK_EQ(odg.operand_var_ids(2), std::vector<char>{'e'});
			CHECK_EQ(odg.operand_var_ids(3), std::vector<char>{'c', 'd'});
			CHECK_EQ(odg.var_ids_positions_in_operands('b'), std::vector<std::vector<pos_type>>{{1}, {0}, {}, {}});
			CHECK_EQ(odg.var_ids_positions_in_operands('c'), std::vector<std::vector<pos_type>>{{}, {1}, {}, {0}});
			// cartesian components
			auto &cart_comps = odg.cartesian_components();
			CHECK_EQ(cart_comps.size(), 2);
			CHECK_EQ(cart_comps[0].size(), 3);
			CHECK_EQ(cart_comps[1].size(), 1);
			CHECK_EQ(cart_comps[0].operands_original_positions(), std::vector<pos_type>{0, 1, 3});
			CHECK_EQ(cart_comps[1].operands_original_positions(), std::vector<pos_type>{2});
			CHECK_EQ(cart_comps[0].operand_var_ids(0), std::vector<char>{'a', 'b'});
			CHECK_EQ(cart_comps[0].operand_var_ids(1), std::vector<char>{'b', 'c'});
			CHECK_EQ(cart_comps[0].operand_var_ids(2), std::vector<char>{'c', 'd'});
			CHECK_EQ(cart_comps[1].operand_var_ids(0), std::vector<char>{'e'});
			// pruning
			auto &pruned1 = odg.prune_graph(std::vector<pos_type>{2});
			CHECK_EQ(pruned1.size(), 0);
			auto &pruned2 = odg.prune_graph(std::vector<pos_type>{0});
			CHECK_EQ(pruned2.size(), 0);
			auto &pruned3 = odg.prune_graph(std::vector<pos_type>{1});
			CHECK_EQ(pruned3.size(), 0);
			auto &pruned4 = odg.prune_graph(std::vector<pos_type>{3});
			CHECK_EQ(pruned4.size(), 0);
			// union components
			CHECK_EQ(odg.union_components().size(), 1);
		}
		SUBCASE("remove label a") {
			odg.isc_operands();// need to instantiate before calling removeLabel
			auto &new_odg = odg.remove_var_id('a');
			CHECK_EQ(new_odg.size(), 4);
			CHECK_EQ(new_odg.operand_var_ids(0), std::vector<char>{'b'});
			CHECK_EQ(odg.operand_var_ids(0), std::vector<char>{'a', 'b'});
		}
		SUBCASE("remove label b") {
			odg.isc_operands();// need to instantiate before calling removeLabel
			auto &new_odg = odg.remove_var_id('b');
			CHECK_EQ(new_odg.size(), 4);
			CHECK_EQ(new_odg.operand_var_ids(0), std::vector<char>{'a'});
			CHECK_EQ(new_odg.operand_var_ids(1), std::vector<char>{'c'});
			CHECK_EQ(odg.operand_var_ids(0), std::vector<char>{'a', 'b'});
			CHECK_EQ(odg.operand_var_ids(1), std::vector<char>{'b', 'c'});
			CHECK_EQ(new_odg.cartesian_components().size(), 2);
			CHECK_EQ(odg.cartesian_components().size(), 2);
		}
		SUBCASE("remove labels b and c") {
			odg.isc_operands();// need instantiate before calling removeLabel
			auto &new_odg = odg.remove_var_id('b');
			auto &new_odg2 = new_odg.remove_var_id('c');
			CHECK_EQ(new_odg2.size(), 3);
			CHECK_EQ(new_odg2.operand_var_ids(0), std::vector<char>{'a'});
			CHECK_EQ(new_odg2.operand_var_ids(1), std::vector<char>{'e'});
			CHECK_EQ(new_odg2.operand_var_ids(2), std::vector<char>{'d'});
			CHECK_EQ(odg.operand_var_ids(0), std::vector<char>{'a', 'b'});
			CHECK_EQ(odg.operand_var_ids(1), std::vector<char>{'b', 'c'});
			CHECK_EQ(new_odg2.cartesian_components().size(), 2);
			CHECK_EQ(odg.cartesian_components().size(), 2);
		}
	}

	TEST_CASE("Independent Strong Component") {
		query::OperandDependencyGraph<htt_t, allocator_type> odg{};
		odg.add_operand({'a', 'b'}, {});
		odg.add_operand({'b', 'c'}, {});
		odg.add_operand({'c', 'd'}, {});
		odg.add_dependency(0, 1, 'b');
		odg.add_dependency(1, 0, 'b');
		odg.add_dependency(1, 2, 'c');
		odg.add_dependency(2, 1, 'c');
		SUBCASE("Strongly Connected") {
			auto const &isc_operands = odg.isc_operands();
			CHECK_EQ(isc_operands.size(), odg.size());
			CHECK_EQ(odg.cartesian_components().size(), 1);
		}
		SUBCASE("Not Strongly Connected (2 Components)") {
			odg.add_operand({'d', 'e'}, {});
			odg.add_operand({'e', 'f'}, {});
			odg.add_dependency(2, 3, 'd');
			odg.add_dependency(3, 4, 'e');
			odg.add_dependency(4, 3, 'e');
			auto const &isc_operands = odg.isc_operands();
			CHECK_EQ(isc_operands, std::vector<pos_type>{0, 1, 2});
			CHECK_EQ(odg.cartesian_components().size(), 1);
		}
		SUBCASE("Not Strongly Connected (3 Components)") {
			odg.add_operand({'d', 'e'}, {});
			odg.add_operand({'e', 'f'}, {});
			odg.add_operand({'f'}, {});
			odg.add_dependency(2, 3, 'd');
			odg.add_dependency(3, 4, 'e');
			odg.add_dependency(4, 3, 'e');
			odg.add_dependency(4, 5, 'f');
			auto const &isc_operands = odg.isc_operands();
			CHECK_EQ(isc_operands, std::vector<pos_type>{0, 1, 2});
			CHECK_EQ(odg.cartesian_components().size(), 1);
		}
	}

	TEST_CASE("Graph with Single Filter") {
		query::OperandDependencyGraph<htt_t, allocator_type> odg{};
		odg.add_operand({'a'}, {});
		odg.add_operand({'a', 'b'}, {});
		odg.add_filter({'a'}, FilterExpression<htt_t, allocator_type>{std::make_unique<TrueExpression>()});
		odg.add_operand({'a', 'c'}, {});
		odg.add_dependency(0, 1, 'a');
		odg.add_dependency(0, 2, 'a');
		odg.add_dependency(0, 3, 'a');
		odg.add_dependency(1, 0, 'a');
		odg.add_dependency(1, 2, 'a');
		odg.add_dependency(1, 3, 'a');
		odg.add_dependency(2, 0, 'a');
		odg.add_dependency(2, 1, 'a');
		odg.add_dependency(2, 3, 'a');
		odg.add_dependency(3, 0, 'a');
		odg.add_dependency(3, 1, 'a');
		odg.add_dependency(3, 2, 'a');
		CHECK(odg.operands_var_ids_set() == boost::container::flat_set{'a', 'b', 'c'});
		CHECK(odg.var_ids_positions_in_operands('a') == std::vector<std::vector<pos_type>>{{0}, {0}, {}, {0}});
		CHECK(odg.var_ids_positions_in_operands('b') == std::vector<std::vector<pos_type>>{{}, {1}, {}, {}});
		CHECK(odg.var_ids_positions_in_operands('c') == std::vector<std::vector<pos_type>>{{}, {}, {}, {1}});
		CHECK(odg.is_filter(2));
		CHECK(not odg.is_filter(0));
		CHECK(odg.filters_for_evaluation().empty());
		SUBCASE("var_id removal - a") {
			auto &sub_odg = odg.remove_var_id('a');
			CHECK(sub_odg.size() == 3);
			CHECK(sub_odg.operands_original_positions() == std::vector<pos_type>{1, 2, 3});
			CHECK(sub_odg.is_filter(1));
			CHECK(not sub_odg.is_filter(2));
			CHECK(sub_odg.operand_var_ids(1) == std::vector<char>{});
			CHECK(sub_odg.var_ids_positions_in_operands('b') == std::vector<std::vector<pos_type>>{{0}, {}, {}});
			CHECK(sub_odg.var_ids_positions_in_operands('c') == std::vector<std::vector<pos_type>>{{}, {}, {0}});
			CHECK(sub_odg.filters_for_evaluation() == std::vector<pos_type>{1});
		}
		SUBCASE("remove filter") {
			auto &sub_odg = odg.remove_vertices(std::vector<pos_type>{2});
			CHECK(sub_odg.operands_var_ids_set() == boost::container::flat_set{'a', 'b', 'c'});
			CHECK(sub_odg.var_ids_positions_in_operands('a') == std::vector<std::vector<pos_type>>{{0}, {0}, {0}});
			CHECK(sub_odg.var_ids_positions_in_operands('b') == std::vector<std::vector<pos_type>>{{}, {1}, {}});
			CHECK(sub_odg.var_ids_positions_in_operands('c') == std::vector<std::vector<pos_type>>{{}, {}, {1}});
			CHECK(sub_odg.filters_for_evaluation().empty());
		}
	}

}// namespace dice::query::tests