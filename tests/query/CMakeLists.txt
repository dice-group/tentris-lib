cmake_minimum_required(VERSION 3.21)

add_executable(tests_OperandDependencyGraph tests_OperandDependencyGraph.cpp)
target_link_libraries(tests_OperandDependencyGraph
        doctest::doctest
        tentris::query
        )
add_test(NAME tests_OperandDependencyGraph COMMAND tests_OperandDependencyGraph)

add_executable(tests_Query tests_Query.cpp)
target_link_libraries(tests_Query
        doctest::doctest
        tentris::query
        tentris::tentris-hypertrie-template-instantiation
        )
add_test(NAME tests_Query COMMAND tests_Query)