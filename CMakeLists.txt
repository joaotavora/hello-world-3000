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
target_link_libraries(Greeter PRIVATE fmt::fmt)
configure_file(include/greeter/version.h.in include/greeter/version.h @ONLY)

file(GLOB sources CONFIGURE_DEPENDS "./src/greeter/*.cpp")
add_executable(GreeterExec ${sources})
set_target_properties(GreeterExec PROPERTIES CXX_STANDARD 20 OUTPUT_NAME
  "Greeter")
target_link_libraries(GreeterExec PRIVATE Greeter cxxopts::cxxopts)

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
find_package(Boost REQUIRED)

if(NOT GREETER_DEV)
  # Installation code goes here, no?
  return()
endif()

#Below this line, dev stuff.  See accompanying README.md.
include(cmake/sanitizers.cmake)
include(cmake/ccache.cmake)
include(cmake/coverage.cmake)

find_package(doctest REQUIRED)

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

include(doctest)
doctest_discover_tests(GreeterTests)

enable_testing()
add_test(NAME GreeterTests COMMAND GreeterTests)
