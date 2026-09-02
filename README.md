
> **⚠️ Note:** This is **not** the commercial version of Tentris, but a research prototype that focuses on aspects presented in papers. For a reliable, SPARQL feature-complete, and well-tested edition, see the commercial edition at https://github.com/tentris/tentris. The Beta is available for free. 


# Tentris library

This repository contains the Tentris library. It provides a generic query package to build custom query processors, a
package for processing SPARQL and a triplestore frontend class.

This repository does not provide any frontend executable, e.g. HTTP or CLI triple stores, tools to work with rdf etc.

# Build

Required tools: clang-14 or gcc-12, CMake 3.21, conan >=1.59,<2.0

You must have [tentris](https://conan.dice-research.org/artifactory/api/conan/tentris) in your conan remotes.

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
