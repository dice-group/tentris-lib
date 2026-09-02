cmake_minimum_required(VERSION 3.21)
set(lib_suffix "metall-node-storage")
set(lib "${PROJECT_NAME}-${lib_suffix}")
# TODO: rename?

# Find cmake packages
find_package(rdf4cpp REQUIRED)
find_package(dice-sparse-map REQUIRED)
find_package(Metall REQUIRED)

# Define the library
add_library(${lib}
        src/dice/metall-node-storage/MetallNodeStorageBackendImpl.cpp
        src/dice/metall-node-storage/MetallNodeStorageBackend.cpp
        src/dice/metall-node-storage/MetallBNodeBackend.cpp
        src/dice/metall-node-storage/MetallIRIBackend.cpp
        src/dice/metall-node-storage/MetallFallbackLiteralBackend.cpp
        src/dice/metall-node-storage/MetallVariableBackend.cpp
        )

add_library(${PROJECT_NAME}::${lib_suffix} ALIAS ${lib})

target_include_directories(${lib} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        )


target_link_libraries(${lib} PUBLIC
        ${PROJECT_NAME}::tentris-param-allocator
        rdf4cpp::rdf4cpp
        dice-sparse-map::dice-sparse-map
        Metall::Metall
        )

include(${CMAKE_SOURCE_DIR}/cmake/install_components.cmake)
install_component(PUBLIC ${lib_suffix} src)
