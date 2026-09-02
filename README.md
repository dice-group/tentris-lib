# Tentris

This repository contains the Tentris library. It provides a generic query package to build custom query processors, a
package for processing SPARQL and a triplestore frontend class.

This repository does not provide any frontend executable, e.g. HTTP or CLI triple stores, tools to work with rdf etc.

# Build

Required tools: clang-14 or gcc-12, CMake 3.21, conan >=1.59,<2.0

You must have [tentris](https://conan.dice-research.org/artifactory/api/conan/tentris)
and [tentris-private](https://conan.dice-research.org/artifactory/api/conan/tentris-private) in your conan remotes.
tentris-private requires credentials to be accessed.

Configure and build it with:

```bash
cmake --build build_dir
cmake -G Ninja -B build_dir
```

Additionally, these CMake options are available:

- `-DBUILD_TESTING=ON/OFF [default: OFF]`: Build the tests.
- `-DBUILD_SHARED_LIBS=ON/OFF [default: OFF]`: Build a shared library instead of a static one.
- `-DUSE_CONAN=ON/OFF [default: ON]`: If available, use Conan to retrieve dependencies.

The Conan packages can be built with:

```bash
conan create . 
```
