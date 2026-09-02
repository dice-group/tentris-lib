cmake_minimum_required(VERSION 3.21)
set(lib_suffix "triplestore")
set(lib "${PROJECT_NAME}-${lib_suffix}")

find_package(robin_hood REQUIRED)

# Define the library
add_library(${lib}
        src/dice/triplestore/TripleStore.cpp
        )

add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

target_include_directories(${lib} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

target_link_libraries(${lib} PUBLIC
        ${PROJECT_NAME}::sparql
        ${PROJECT_NAME}::metall-node-storage
        ${PROJECT_NAME}::logger
        robin_hood::robin_hood
        )

## Packaging and exporting the target
include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(PUBLIC ${lib_suffix} src)
