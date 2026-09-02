cmake_minimum_required(VERSION 3.21)
set(lib_suffix "query")
set(lib "${PROJECT_NAME}-${lib_suffix}")

find_package(hypertrie REQUIRED COMPONENTS hypertrie)
find_package(robin_hood REQUIRED)

add_library(${lib} INTERFACE)
add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

target_include_directories(${lib} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

target_link_libraries(${lib} INTERFACE
        robin_hood::robin_hood
        hypertrie::hypertrie
        )


include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(INTERFACE ${lib_suffix} src)
