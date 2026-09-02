cmake_minimum_required(VERSION 3.21)
project(tentris VERSION 3.0.0
        DESCRIPTION "The library powering Tentris. Batteries (hypertrie) are elsewhere.")

include(cmake/boilerplate_init.cmake)
boilerplate_init()

OPTION(BUILD_FFI "Build Tentris foreign function interface" OFF)
OPTION(WITH_HYPERTRIE_FFI "Build Tentris foreign function interface with hypertrie support" OFF)

OPTION(USE_CONAN "If available, use conan to retrieve dependencies." ON)
if (PROJECT_IS_TOP_LEVEL AND USE_CONAN)
    include(cmake/conan_cmake.cmake)
    if (PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
        set(CONAN_TENTRIS_WITH_TEST_DEPS "True")
    else()
        set(CONAN_TENTRIS_WITH_TEST_DEPS "False")
    endif()

    if (BUILD_FFI)
        set(CONAN_TENTRIS_BUILD_FFI "True")
    else ()
        set(CONAN_TENTRIS_BUILD_FFI "False")
    endif ()

    if (WITH_HYPERTRIE_FFI)
        set(CONAN_TENTRIS_WITH_HYPERTRIE_FFI "True")
    else ()
        set(CONAN_TENTRIS_WITH_HYPERTRIE_FFI "False")
    endif ()

    set(CONAN_OPTIONS "with_test_deps=${CONAN_TENTRIS_WITH_TEST_DEPS};build_ffi=${CONAN_TENTRIS_BUILD_FFI};with_hypertrie_ffi=${CONAN_TENTRIS_WITH_HYPERTRIE_FFI};boost:header_only=True")
    install_packages_via_conan("${CMAKE_SOURCE_DIR}/conanfile.py" "${CONAN_OPTIONS}")
endif ()

if (PROJECT_IS_TOP_LEVEL AND USE_CLANG_TIDY)
    include(cmake/ClangTidy.cmake)
endif ()

add_subdirectory(libs)

if (PROJECT_IS_TOP_LEVEL AND BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()
