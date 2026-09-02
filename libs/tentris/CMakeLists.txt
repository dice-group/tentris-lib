cmake_minimum_required(VERSION 3.21)
# meta library tentris::tentris
# imports everything and holds the version header
set(lib_suffix "${PROJECT_NAME}")
set(lib "${PROJECT_NAME}-${lib_suffix}")

# Define the library
add_library(${lib} INTERFACE)
add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/dice/${PROJECT_NAME}/${PROJECT_NAME}_version.hpp)

target_include_directories(${lib} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

target_link_libraries(${lib} INTERFACE
        ${PROJECT_NAME}::triplestore
        )

## Packaging and exporting the target
include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(INTERFACE ${lib_suffix} src)
