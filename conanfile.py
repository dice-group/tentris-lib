import os
import re

from conan import ConanFile
from conan import tools
from conan.tools.cmake import CMake
from conan.tools.files import load


class Recipe(ConanFile):
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_test_deps": [True, False],
        "build_ffi": [True, False],
        "with_hypertrie_ffi": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_test_deps": False,
        "build_ffi": False,
        "with_hypertrie_ffi": False,
    }
    exports_sources = "libs/*", "CMakeLists.txt", "cmake/*"
    generators = ("CMakeDeps", "CMakeToolchain")

    def requirements(self):
        self.requires("hypertrie/0.15.0")
        self.requires("rdf4cpp/0.0.18")
        self.requires("metall/0.23.1")
        self.requires("sparql-parser-base/0.3.1")
        self.requires("boost/1.81.0")
        self.requires("dice-hash/0.4.3")
        self.requires("dice-sparse-map/0.2.4")
        self.requires("robin-hood-hashing/3.11.5")

        if self.options.with_test_deps:
            self.requires("openssl/3.0.8")
            self.requires("doctest/2.4.9")
            self.requires("pugixml/1.12.1")
            self.requires("libcurl/7.85.0")

        if self.options.build_ffi:
            self.requires("metall-ffi/0.2.1")

            if self.options.with_hypertrie_ffi:
                self.options["hypertrie"].build_ffi = True

    def set_name(self):
        if not hasattr(self, 'name') or self.version is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.name = re.search(r"project\(\s*([a-z\-]+)\s+VERSION", cmake_file).group(1)

    def set_version(self):
        if not hasattr(self, 'version') or self.version is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.version = re.search(r"project\([^)]*VERSION\s+(\d+\.\d+.\d+)[^)]*\)", cmake_file).group(1)
        if not hasattr(self, 'description') or self.description is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.description = re.search(r"project\([^)]*DESCRIPTION\s+\"([^\"]+)\"[^)]*\)", cmake_file).group(1)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    _cmake = None

    def _configure_cmake(self):
        if self._cmake is None:
            self._cmake = CMake(self)
            self._cmake.configure(variables={"USE_CONAN": False, "BUILD_FFI": self.options.build_ffi, "WITH_HYPERTRIE_FFI": self.options.with_hypertrie_ffi})

        return self._cmake

    def build(self):
        self._configure_cmake().build()

    def package(self):
        self._configure_cmake().install()
        print("dir: " + str(os.path.join(self.package_folder, "cmake")))
        tools.files.rmdir(self, os.path.join(self.package_folder, "cmake"))
        tools.files.rmdir(self, os.path.join(self.package_folder, "share"))
        tools.files.copy(self, "LICENSE", src=self.folders.base_source, dst="licenses")

    def package_info(self):  #
        main_component = self.name
        self.cpp_info.set_property("cmake_target_name", f"{self.name}")
        self.cpp_info.components["global"].set_property("cmake_target_name", f"{self.name}::{main_component}")
        self.cpp_info.components["global"].names["cmake_find_package_multi"] = f"{self.name}"
        self.cpp_info.components["global"].names["cmake_find_package"] = f"{self.name}"
        self.cpp_info.set_property("cmake_file_name", f"{self.name}")
        self.cpp_info.components["global"].includedirs = [f"include/{self.name}/{main_component}/"]

        if self.options.with_test_deps:
            self.cpp_info.components["global"].requires.append("doctest::doctest")
            self.cpp_info.components["global"].requires.append("pugixml::pugixml")
            self.cpp_info.components["global"].requires.append("libcurl::libcurl")

        self.cpp_info.components["logger"].requires = ()

        self.cpp_info.components["node-wrapper"].requires = (
            "rdf4cpp::rdf4cpp",
            "dice-hash::dice-hash"
        )

        self.cpp_info.components["tentris-param-allocator"].requires = (
            "boost::headers",
            "metall::metall"
        )

        self.cpp_info.components["tentris-hypertrie-template-instantiation"].requires = (
            "node-wrapper",
            "tentris-param-allocator",
            "hypertrie::hypertrie",
        )

        self.cpp_info.components["metall-node-storage"].requires = (
            "tentris-param-allocator",
            "rdf4cpp::rdf4cpp",
            "dice-sparse-map::dice-sparse-map",
        )

        self.cpp_info.components["query"].requires = (
            "hypertrie::hypertrie",
            "robin-hood-hashing::robin-hood-hashing",
        )

        self.cpp_info.components["sparql"].requires = (
            "query",
            "node-wrapper",
            "tentris-hypertrie-template-instantiation",
            "sparql-parser-base::sparql-parser-base",
            "robin-hood-hashing::robin-hood-hashing",
        )

        self.cpp_info.components["triplestore"].requires = (
            "sparql",
            "metall-node-storage",
            "logger",
            "robin-hood-hashing::robin-hood-hashing",
        )

        if self.options.build_ffi:
            self.cpp_info.components["ffi"].requires = (
                "triplestore",
                "logger",
                "metall-ffi::metall-ffi",
            )

            if self.options.with_hypertrie_ffi:
                self.cpp_info.components["ffi"].requires += "hypertrie::ffi",

        for component in ["metall-node-storage", "node-wrapper", "query", "sparql",
                          "tentris-hypertrie-template-instantiation", "tentris-param-allocator",
                          "triplestore", "logger", "ffi"]:
            self.cpp_info.components[f"{component}"].includedirs = [f"include/{self.name}/{component}"]
            self.cpp_info.components[f"{component}"].names["cmake_find_package_multi"] = f"{component}"
            self.cpp_info.components[f"{component}"].names["cmake_find_package"] = f"{component}"

        for component in ["sparql", "metall-node-storage", "triplestore", "tentris-hypertrie-template-instantiation"] + (["ffi"] if self.options.build_ffi else []):
            self.cpp_info.components[f"{component}"].libdirs = [f"lib/{self.name}/{component}"]
            self.cpp_info.components[f"{component}"].libs = [f"{self.name}-{component}"]

        self.cpp_info.components["global"].requires = [
            "triplestore",
        ]
