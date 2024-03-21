cmake_minimum_required(VERSION 3.14...3.22)
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
set_target_properties(Greeter PROPERTIES CXX_STANDARD 17)
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
set_target_properties(GreeterExec PROPERTIES CXX_STANDARD 17 OUTPUT_NAME
  "Greeter")
target_link_libraries(GreeterExec
  PRIVATE Greeter cxxopts::cxxopts Boost::asio)

if(NOT GREETER_DEV)
  find_package(cxxopts REQUIRED)
  find_package(fmt REQUIRED)
  find_package(spdlog REQUIRED)

  # Boost needs this, since find_package(Boost) doesn't bring in Cmake
  # targets like 'Boost::asio' for its header-only libraries, but
  # CPM-fetched Boost does.  So, in the interest of referring to these
  # targets by the same names, consitently.  Make a dummy target and
  # alias to it.
  find_package(Boost REQUIRED)
  add_library(BoostDummy INTERFACE)
  target_link_libraries(BoostDummy INTERFACE)
  add_library(Boost::asio ALIAS BoostDummy)

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

if(CMAKE_BUILD_TYPE EQUAL "Debug")
  message("USE_SANITIZER is '${USE_SANITIZER}'")
  CPMAddPackage(
    "gh:StableCoder/cmake-scripts#1f822d1fc87c8d7720c074cde8a278b44963c354")
  # There are more tools here, but let's just use the sanitizer
  include(${cmake-scripts_SOURCE_DIR}/sanitizers.cmake)
endif()

CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.3")

CPMAddPackage(
  NAME fmt GIT_TAG 10.2.1 GITHUB_REPOSITORY fmtlib/fmt
)

CPMAddPackage(
  NAME Boost
  VERSION 1.84.0
  URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
  URL_HASH SHA256=2e64e5d79a738d0fa6fb546c6e5c2bd28f88d268a2a080546f74e5ff98f29d0e
  OPTIONS "BOOST_ENABLE_CMAKE ON" "BOOST_INCLUDE_LIBRARIES process\\\;asio\\\;process" # Note the escapes!
)

CPMAddPackage(
  GITHUB_REPOSITORY jarro2783/cxxopts VERSION 3.0.0
  OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO"
  "CXXOPTS_GREETER_INSTALL YES")

CPMAddPackage("gh:gabime/spdlog@1.8.2")

option(GREETER_COVERAGE "Enable test coverage" OFF)

CPMAddPackage("gh:doctest/doctest@2.4.9")

file(GLOB sources CONFIGURE_DEPENDS "./test/*.cpp")
add_executable(GreeterTests ${sources})
target_link_libraries(GreeterTests doctest::doctest Greeter Boost::asio Boost::process)
set_target_properties(GreeterTests PROPERTIES CXX_STANDARD 17)

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

if(GREETER_COVERAGE)
  target_compile_options(Greeter PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(Greeter PUBLIC -fprofile-arcs -ftest-coverage)
endif()
