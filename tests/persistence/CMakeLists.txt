find_package(Metall REQUIRED)

add_executable(Persistence_phase1 Persistence_phase1.cpp)
target_link_libraries(Persistence_phase1
        PRIVATE
        tentris::triplestore
        test_logger
        )

add_executable(Persistence_phase2 Persistence_phase2.cpp)
target_link_libraries(Persistence_phase2
        PRIVATE
        tentris::triplestore
        test_logger
        )

add_executable(tests_Persistence tests_Persistence.cpp)
add_dependencies(tests_Persistence Persistence_phase1 Persistence_phase2)
target_link_libraries(tests_Persistence
        PRIVATE
        doctest::doctest
        )
add_test(NAME tests_Persistence COMMAND tests_Persistence)
