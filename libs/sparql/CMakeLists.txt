cmake_minimum_required(VERSION 3.21)
set(lib_suffix "sparql")
set(lib "${PROJECT_NAME}-${lib_suffix}")

find_package(sparql-parser-base REQUIRED)
find_package(robin_hood REQUIRED)

add_library(${lib}
        src/dice/sparql/expressions/UnaryOperators.cpp
        src/dice/sparql/expressions/BuiltInCalls.cpp
        src/dice/sparql/expressions/LogicalOperators.cpp
        src/dice/sparql/expressions/PrimaryExpressions.cpp
        src/dice/sparql/expressions/Aggregates.cpp
        src/dice/sparql/expressions/Exists.cpp
        src/dice/sparql/parser/SPARQLParser.cpp
        src/dice/sparql/parser/visitors/PrologueVisitor.cpp
        src/dice/sparql/parser/visitors/QueryUpdateVisitor.cpp
        src/dice/sparql/parser/exception/Exceptions.cpp
        src/dice/sparql/parser/exception/SPARQLErrorListener.cpp
        src/dice/sparql/SPARQLQuery.cpp
        src/dice/sparql/expressions/ArithmeticExpressions.cpp
        src/dice/sparql/expressions/functions/XSDFunctions.cpp
        )
add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

target_include_directories(${lib} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private-include
        )

target_link_libraries(${lib} PUBLIC
        ${PROJECT_NAME}::query
        ${PROJECT_NAME}::node-wrapper
        ${PROJECT_NAME}::tentris-hypertrie-template-instantiation
        robin_hood::robin_hood
        PRIVATE
        sparql-parser-base::sparql-parser-base
        )

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(PUBLIC ${lib_suffix} src)
