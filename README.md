[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/MacOS/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/Windows/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/Ubuntu/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/Style/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/Install/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![codecov](https://codecov.io/gh/joaotavora/hello-world-3000/branch/master/graph/badge.svg)](https://codecov.io/gh/joaotavora/hello-world-3000)

# Hello World 3000

Modern C++ project template forked from
[ModernCppStarter][modern-cpp-starter], but without pushing
[CPM.cmake][cpm-cmake] so much.

## Dual-Use CMakeLists.txt

`CMakeLists.txt` is crafted to serve both developers and
non-developers.

- For non-developer end users who want to try out the library or the
  program it ensures that they can build and run the project in typical
  fashion for a *nix project.  Download the code, `make configure`,
  `make build`, `make install` (the latter isn't supported yet).  For
  
- For developer end users who want to link against this library by
  bundling it into their project however they see fit (git submodule,
  CPM.cmake, package manager), they should in theory only need to call
  'add_subdirectory()' and they will have the the Greeter, GreeterExec
  targets available.

- For developers of the library, on the other hand, there are
  additional features like sanitizers, unit testing, `ccache`, code
  coverage dependency management through [CPM.cmake][cpm-cmake].  For
  now, I find this easier than Conan or vcpkg.

The project is set up to encourage Test-Driven Development (TDD) just
like [ModernCppStarter][modern-cpp-starter].

## Project Structure

* `/src` and `/include`: Contain the source and header files,
  respectively.

* `/test`: Houses test cases, promoting TDD from the start.

* `/doc`: For project documentation, supporting immediate and easy
  documentation generation.

* `CMakeLists.txt`: Configures both developer and non-developer
  builds, ensuring that the project can be easily built in different
  environments and setups.

* `Makefile`: Provides convenient shortcuts for common tasks, tailored
  for both developers and non-developers.

## Getting Started 

This part of the README could be kept for whatever project you make of
this.

### Prerequisites

Ensure the following are installed:

- CMake (version 3.14 or later)
- Make
- A C++17 compatible compiler

If you plan to develop the project, that should be enough if you plan
to make use of (say, build and install linking to your system
libraries) you need the necessary project dependencies (e.g., Boost,
fmt).

### Building the Project

1. **For Non-Developers**: 
   - Execute `make build` for building the project.
   - Use `make install` to install (currently not supported).

2. **For Developers**: 
   - Use `make configure-release` or `make configure-debug` for configuring the project.
   - Build with `make build-release` or `make build-debug`.

### Running Tests

- Run `make check-debug` to execute unit tests for the debug
  configuration.  `make watch-debug` is nice, too.  `check-release`
  and `watch-release` should also work of course.

### Generating Documentation

- Execute `make doc` to generate documentation.

### Code Formatting

- Use `make check-format` for checking and `make fix-format` for
  fixing the code format.

## License

TBD.  Again, this is forked from
[ModernCppStarter][modern-cpp-starter], which is the public domain, so
probably this will too.

[modern-cpp-starter]: https://github.com/TheLartians/ModernCppStarter
[cpm-cmake]: https://github.com/cpm-cmake/CPM.cmake
