cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
# code coverage
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if ("${CMAKE_C_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "(Apple)?[Cc]lang")
        message("Building with llvm Code Coverage Tools")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")

    elseif (CMAKE_COMPILER_IS_GNUCXX)
        message("Building with lcov Code Coverage Tools")

        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    endif ()
endif ()

find_package(doctest REQUIRED)

add_library(test_logger TestLogger.cpp)
target_link_libraries(test_logger
        PUBLIC
        tentris::logger
)

add_subdirectory(query)
add_subdirectory(sparql)
add_subdirectory(triplestore)
add_subdirectory(persistence)

if (BUILD_FFI)
    add_subdirectory(ffi)
endif ()
