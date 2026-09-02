cmake_minimum_required(VERSION 3.21)
set(lib_suffix "tentris-param-allocator")
set(lib "${PROJECT_NAME}-${lib_suffix}")

# Find cmake packages
find_package(Boost REQUIRED COMPONENTS)
find_package(Metall REQUIRED)
find_package(dice-template-library REQUIRED)


# Define the library
add_library(${lib} INTERFACE)
add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

target_link_libraries(${lib} INTERFACE
        Boost::headers
        Metall::Metall
        dice-template-library::dice-template-library
        )

target_include_directories(${lib} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(INTERFACE ${lib_suffix} src)