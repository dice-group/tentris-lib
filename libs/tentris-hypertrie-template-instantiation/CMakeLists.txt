cmake_minimum_required(VERSION 3.21)
set(lib_suffix "tentris-hypertrie-template-instantiation")
set(lib "${PROJECT_NAME}-${lib_suffix}")

# Find cmake packages
find_package(hypertrie REQUIRED)
find_package(dice-hash REQUIRED)

# Define the library
add_library(${lib} src/dice/tentris/hypertrie-template-instantiation.cpp)
add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

set_target_properties(${lib} PROPERTIES CXX_STANDARD 20)

target_link_libraries(${lib} PUBLIC
        ${PROJECT_NAME}::tentris-param-allocator
        ${PROJECT_NAME}::node-wrapper
        hypertrie::hypertrie
        )

target_include_directories(${lib} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(PUBLIC ${lib_suffix} src)
