add_executable(tests_triplestore_NodeStorage tests_NodeStorage.cpp)
target_link_libraries(tests_triplestore_NodeStorage
        PRIVATE
        doctest::doctest
        tentris::triplestore
        test_logger
        )
add_test(NAME tests_triplestore_NodeStorage COMMAND tests_triplestore_NodeStorage)

add_executable(tests_triplestore_InsertDelete tests_InsertDelete.cpp)
target_link_libraries(tests_triplestore_InsertDelete
        PRIVATE
        doctest::doctest
        tentris::triplestore
        test_logger
        )
add_test(NAME tests_triplestore_InsertDelete COMMAND tests_triplestore_InsertDelete)

add_executable(tests_triplestore_Construct tests_Construct.cpp)
target_link_libraries(tests_triplestore_Construct
        PRIVATE
        doctest::doctest
        tentris::triplestore
        test_logger
        )
add_test(NAME tests_triplestore_Construct COMMAND tests_triplestore_Construct)

add_executable(tests_triplestore_Dangling tests_Dangling.cpp)
target_link_libraries(tests_triplestore_Dangling
        PRIVATE
        doctest::doctest
        tentris::triplestore
        test_logger
        )
add_test(NAME tests_triplestore_Dangling COMMAND tests_triplestore_Dangling)

add_executable(tests_triplestore_Timeout tests_Timeout.cpp)
target_link_libraries(tests_triplestore_Timeout
        doctest::doctest
        tentris::triplestore
        test_logger
        )
add_test(NAME tests_triplestore_Timeout COMMAND tests_triplestore_Timeout)
