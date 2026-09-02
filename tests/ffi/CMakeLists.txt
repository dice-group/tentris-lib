add_executable(tests_FFI tests_FFI.cpp)
target_link_libraries(tests_FFI
        PRIVATE
        tentris::ffi
        doctest::doctest
        test_logger
        )
add_test(NAME tests_FFI COMMAND tests_FFI)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/test_data/swdf_10.nt" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
