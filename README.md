[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/MacOS/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/Windows/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/Ubuntu/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/Style/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![Actions Status](https://github.com/joaotavora/hello-world-3000/workflows/Install/badge.svg)](https://github.com/joaotavora/hello-world-3000/actions)
[![codecov](https://codecov.io/gh/joaotavora/hello-world-3000/branch/master/graph/badge.svg)](https://codecov.io/gh/joaotavora/hello-world-3000)

# Hello World 3000

Modern C++ project template forked from
[ModernCppStarter][modern-cpp-starter], but trying to push
[CPM.cmake][cpm-cmake] a bit less.

## Dual-Use CMakeLists.txt

`CMakeLists.txt` is crafted to different consumers:

- End users who want to try out the library or the program.  They can
  build and run the project in the classic fashion of `make && make
  install` (the latter isn't supported yet, tho).

- This project's devs who want to hack on it.  Here there are features
  like sanitizers, unit testing, `ccache`, code coverage.  This is
  more or less C++ package manager agnostic.  It uses `find_package`
  for C++ packages, so install these using your system package manager
  or Conan or something or find some way to make that work :-)
  
- The CI, who is a bit like a special project dev.
  
- Other project devs who want "vendor" this library with their project
  however they see fit (`git submodule`, `FetchContent`, other). In
  theory they need call `add_subdirectory(<this very dir>)` and they
  will have the Greeter, GreeterExec targets available to link against
  in their `CMakeLists.txt`.

## Project Structure

* `/src` and `/include`: Contain the source and header files,
  respectively.

* `/test`: Houses test cases, promoting TDD from the start.

* `/doc`: For project documentation, supporting immediate and easy
  documentation generation.

* `/.github`: GitHub CI stuff.

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

1. **For users**: 
   - Execute `make build` for building the project.
   - Use `make install` to install (currently not supported).

2. **For developers**: 
   - Use `make configure-release` or `make configure-debug` for configuring the project.
   - Build with `make build-release` or `make build-debug`.
   - Run tests with `make check-debug`.  `CTEST_OPTIONS` may be used
     to pass additional things to `ctest`, like test-filtering
     regexps.
   - Run coverage with `make coverage-debug`.
   - See `Makefile` for more tests

3. **For other projects wanting to vendor this project**
   - TODO

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
