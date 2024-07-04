cmake_minimum_required(VERSION 3.14...3.30)
project(Greeter VERSION 1.0.0 LANGUAGES CXX)

if(GREETER_DOCS)
  add_subdirectory("./doc")
  return()
endif()

if(GREETER_DEV)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "./include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "./src/libgreeter/*.cpp")
add_library(Greeter ${headers} ${sources})
set_target_properties(Greeter PROPERTIES CXX_STANDARD 20)
target_compile_options(
  Greeter PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_include_directories(
  Greeter
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
target_link_libraries(Greeter PRIVATE fmt::fmt Boost::asio)
configure_file(include/greeter/version.h.in include/greeter/version.h @ONLY)

file(GLOB sources CONFIGURE_DEPENDS "./src/greeter/*.cpp")
add_executable(GreeterExec ${sources})
set_target_properties(GreeterExec PROPERTIES CXX_STANDARD 20 OUTPUT_NAME
  "Greeter")
target_link_libraries(GreeterExec PRIVATE Greeter cxxopts::cxxopts Boost::asio)

if(NOT GREETER_DEV)
  find_package(cxxopts REQUIRED)
  find_package(fmt REQUIRED)

  # Boost needs this, since find_package(Boost) doesn't bring in Cmake
  # targets like 'Boost::asio' for its header-only libraries, but
  # CPM-fetched Boost does.  So, in the interest of referring to these
  # targets by the same names, consitently, make a dummy target and
  # alias to it.
  find_package(Boost REQUIRED)
  add_library(BoostDummy INTERFACE)
  target_link_libraries(BoostDummy INTERFACE)
  add_library(Boost::asio ALIAS BoostDummy)
  # add_library(Boost::beast ALIAS BoostDummy)

  # Installation code will come here.  For now that important target
  # is completely absent..

  return()
endif()

# Up to here, everything is more or less standard "idiomatic" CMake
# for non-developers.  Below this line, dev stuff.  See accompanying
# README.md.
set(CPM_USE_NAMED_CACHE_DIRECTORIES ON)
set(CPM_SOURCE_CACHE "$ENV{HOME}/.cache/CPM")
include(cmake/CPM.cmake)
include(cmake/sanitizers.cmake)
include(cmake/ccache.cmake)
include(cmake/coverage.cmake)

CPMAddPackage(
  NAME fmt GIT_TAG 10.2.1 GITHUB_REPOSITORY fmtlib/fmt
)
CPMAddPackage(
  NAME Boost
  VERSION 1.84.0
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
  URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
  OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_INCLUDE_LIBRARIES process\\\;asio" # Note the escapes!
)
CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts VERSION 3.0.0
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO"
  "CXXOPTS_GREETER_INSTALL YES")

CPMAddPackage("gh:doctest/doctest@2.4.9")

file(GLOB sources CONFIGURE_DEPENDS "./test/*.cpp")
add_executable(GreeterTests ${sources})
target_link_libraries(GreeterTests doctest::doctest Greeter)
set_target_properties(GreeterTests PROPERTIES CXX_STANDARD 20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES
    "GNU")
  target_compile_options(Greeter PUBLIC -Wall -Wpedantic -Wextra -Werror)
elseif(MSVC)
  target_compile_options(Greeter PUBLIC /W4 /WX)
  target_compile_definitions(Greeter PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
endif()

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(GreeterTests)

enable_testing()
add_test(NAME GreeterTests COMMAND GreeterTests)

