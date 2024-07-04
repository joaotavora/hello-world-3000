cmake_minimum_required(VERSION 3.14...3.30)
project(Greeter VERSION 1.0.0 LANGUAGES CXX)

include(cmake/prelude.cmake)

if(Greeter_DOCS)
  add_subdirectory("./doc")
  return()
endif()

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "./include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "./src/libgreeter/*.cpp")
add_library(Greeter ${headers} ${sources})
target_compile_options(
  Greeter PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
target_include_directories(
  Greeter
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)
target_link_libraries(Greeter PRIVATE fmt::fmt)
configure_file(include/greeter/version.h.in include/greeter/version.h @ONLY)

file(GLOB sources CONFIGURE_DEPENDS "./src/greeter/*.cpp")
add_executable(GreeterExec ${sources})
set_target_properties(GreeterExec PROPERTIES OUTPUT_NAME "Greeter")
target_link_libraries(GreeterExec PRIVATE Greeter cxxopts::cxxopts)

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
# find_package(Boost REQUIRED)

get_target_property(hmmm fmt::fmt INCLUDE_DIRECTORIES)
message(STATUS "Very strange ${hmmm}")


include(cmake/compiler-features.cmake)

if(NOT Greeter_DEV)
  # Installation code goes here, no?
  return()
endif()

#Below this line, dev stuff.  See accompanying README.md.
include(cmake/sanitizers.cmake)
include(cmake/ccache.cmake)
include(cmake/coverage.cmake)
include(cmake/compiler-flags.cmake)

find_package(doctest REQUIRED)

file(GLOB sources CONFIGURE_DEPENDS "./test/*.cpp")
add_executable(GreeterTests ${sources})
target_link_libraries(GreeterTests doctest::doctest Greeter)

include(doctest)
doctest_discover_tests(GreeterTests)

enable_testing()
add_test(NAME GreeterTests COMMAND GreeterTests)
