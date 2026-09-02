# use std::allocator instead of metall for the tests

find_package(doctest REQUIRED)
find_package(pugixml REQUIRED)
find_package(CURL REQUIRED)

add_library(tests_Commons tests_Commons.cpp)
target_link_libraries(tests_Commons
        PUBLIC
        doctest::doctest
        tentris::sparql
        PRIVATE
        tentris::metall-node-storage
        pugixml::pugixml
        CURL::libcurl
        )

add_executable(tests_SPARQLParsing tests_SPARQLParsing.cpp)
target_link_libraries(tests_SPARQLParsing
        PRIVATE
        doctest::doctest
        tentris::sparql
        )
add_test(NAME tests_SPARQLParsing COMMAND tests_SPARQLParsing)

add_executable(tests_SPARQL_Aggregates tests_Aggregates.cpp)
target_link_libraries(tests_SPARQL_Aggregates
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Aggregates COMMAND tests_SPARQL_Aggregates)


add_executable(tests_SPARQL_Algebra tests_Algebra.cpp)
target_link_libraries(tests_SPARQL_Algebra
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Algebra COMMAND tests_SPARQL_Algebra)

add_executable(tests_SPARQL_Subqueries tests_Subqueries.cpp)
target_link_libraries(tests_SPARQL_Subqueries
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Subqueries COMMAND tests_SPARQL_Subqueries)

add_executable(tests_SPARQL_Exists tests_Exists.cpp)
target_link_libraries(tests_SPARQL_Exists
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Exists COMMAND tests_SPARQL_Exists)

add_executable(tests_SPARQL_Functions tests_Functions.cpp)
target_link_libraries(tests_SPARQL_Functions
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Functions COMMAND tests_SPARQL_Functions)

add_executable(tests_SPARQL_ProjectExpressions tests_ProjectExpressions.cpp)
target_link_libraries(tests_SPARQL_ProjectExpressions
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_ProjectExpressions COMMAND tests_SPARQL_ProjectExpressions)

add_executable(tests_SPARQL_Inline tests_Inline.cpp)
target_link_libraries(tests_SPARQL_Inline
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Inline COMMAND tests_SPARQL_Inline)

add_executable(tests_SPARQL_SolutionSequence tests_SolutionSequence.cpp)
target_link_libraries(tests_SPARQL_SolutionSequence
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_SolutionSequence COMMAND tests_SPARQL_SolutionSequence)

add_executable(tests_SPARQL_ExpressionOperations tests_ExpressionOperations.cpp)
target_link_libraries(tests_SPARQL_ExpressionOperations
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_ExpressionOperations COMMAND tests_SPARQL_ExpressionOperations)

add_executable(tests_SPARQL_Cast tests_Cast.cpp)
target_link_libraries(tests_SPARQL_Cast
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Cast COMMAND tests_SPARQL_Cast)

add_executable(tests_SPARQL_PropertyPaths tests_PropertyPaths.cpp)
target_link_libraries(tests_SPARQL_PropertyPaths
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_PropertyPaths COMMAND tests_SPARQL_PropertyPaths)

add_executable(tests_SPARQL_Construct tests_Construct.cpp)
target_link_libraries(tests_SPARQL_Construct
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Construct COMMAND tests_SPARQL_Construct)

add_executable(tests_SPARQL_Bind tests_Bind.cpp)
target_link_libraries(tests_SPARQL_Bind
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_Bind COMMAND tests_SPARQL_Bind)

add_executable(tests_SPARQL_DeleteWhere tests_DeleteWhere.cpp)
target_link_libraries(tests_SPARQL_DeleteWhere
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_DeleteWhere COMMAND tests_SPARQL_DeleteWhere)

add_executable(tests_SPARQL_DeleteInsert tests_DeleteInsert.cpp)
target_link_libraries(tests_SPARQL_DeleteInsert
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_DeleteInsert COMMAND tests_SPARQL_DeleteInsert)

add_executable(tests_SPARQL_DeleteData tests_DeleteData.cpp)
target_link_libraries(tests_SPARQL_DeleteData
        PRIVATE
        tests_Commons
        )
add_test(NAME tests_SPARQL_DeleteData COMMAND tests_SPARQL_DeleteData)
