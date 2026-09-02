cmake_minimum_required(VERSION 3.21)
set(lib_suffix "ffi")
set(lib "${PROJECT_NAME}-${lib_suffix}")

find_package(metall-ffi REQUIRED)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/check_type_size.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/check_type_size.cpp @ONLY)

## This executable is used to configure tentris.h.in
add_executable(${lib}_check_type_size ${CMAKE_CURRENT_BINARY_DIR}/check_type_size.cpp)

target_link_libraries(${lib}_check_type_size
        PRIVATE
        ${PROJECT_NAME}::triplestore
        )

target_include_directories(${lib}_check_type_size
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private-include>
        )

set_target_properties(${lib}_check_type_size
        PROPERTIES
        CXX_STANDARD 20
        )

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generating-done.cpp
        COMMAND ${lib}_check_type_size
        MAIN_DEPENDENCY src/dice/ffi/tentris.h.in
        DEPENDS ${lib}_check_type_size
        VERBATIM)


add_library(${lib}
        src/dice/ffi/tentris.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/generating-done.cpp
        )

add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

add_dependencies(${lib} ${lib}_check_type_size)


if (WITH_HYPERTRIE_FFI)
        find_package(hypertrie REQUIRED COMPONENTS ffi)

        target_link_libraries(${lib}
                PUBLIC
                hypertrie::ffi
        )

        target_compile_definitions(${lib}
                PUBLIC
                TENTRIS_FFI_WITH_HYPERTRIE=1
        )
endif ()


target_link_libraries(${lib}
        PUBLIC
        ${PROJECT_NAME}::triplestore
        ${PROJECT_NAME}::logger
        metall-ffi::metall-ffi
        )

target_include_directories(${lib}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private-include>
        )

set_target_properties(${lib}
        PROPERTIES
        CXX_STANDARD 20
        )


## This executable can be used by consumers of this library to find which static libraries they need to link against
add_executable(${lib}_link_detect link_detect.cpp)
target_link_libraries(${lib}_link_detect
        PUBLIC
        ${lib}
        )


## Packaging and exporting the target
include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(PUBLIC ${lib_suffix} src)
