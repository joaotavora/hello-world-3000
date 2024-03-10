cmake_minimum_required(VERSION 3.14...3.22)

project(Greeter VERSION 1.0.0 LANGUAGES CXX)

if(GREETER_DEV OR GREETER_DOCS)
  set(CPM_USE_NAMED_CACHE_DIRECTORIES ON)
  set(CPM_SOURCE_CACHE "$ENV{HOME}/.cache/CPM")
  include(cmake/CPM.cmake)
endif()

if(GREETER_DOCS)
  CPMAddPackage("gh:mosra/m.css#a0d292ec311b97fefd21e93cdefb60f88d19ede6")

  set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
  configure_file(./doc/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
  configure_file(./doc/conf.py ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

  add_custom_target(
    GreeterDocs
    ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
    COMMAND "${m.css_SOURCE_DIR}/documentation/doxygen.py"
            "${CMAKE_CURRENT_BINARY_DIR}/conf.py"
    COMMAND echo "Docs written to: ${DOXYGEN_OUTPUT_DIRECTORY}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  return()
endif()

file(GLOB_RECURSE headers CONFIGURE_DEPENDS "./include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "./src/libgreeter/*.cpp")
add_library(Greeter ${headers} ${sources})
set_target_properties(Greeter PROPERTIES CXX_STANDARD 17)
target_compile_options(
  Greeter PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

file(GLOB sources CONFIGURE_DEPENDS "./src/greeter/*.cpp")
add_executable(GreeterExec ${sources})
set_target_properties(GreeterExec PROPERTIES CXX_STANDARD 17 OUTPUT_NAME
  "Greeter")
target_link_libraries(GreeterExec PRIVATE Greeter cxxopts::cxxopts)

configure_file(include/greeter/version.h.in include/greeter/version.h @ONLY)

target_include_directories(
  Greeter
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

target_link_libraries(Greeter PRIVATE fmt::fmt)

if(GREETER_DEV)
  if(CMAKE_BUILD_TYPE EQUAL "Debug")
    message("USE_SANITIZER is '${USE_SANITIZER}'")
    CPMAddPackage(
      "gh:StableCoder/cmake-scripts#1f822d1fc87c8d7720c074cde8a278b44963c354")
    # There are more tools here, but let's just use the sanitizer
    include(${cmake-scripts_SOURCE_DIR}/sanitizers.cmake)
  endif()

  CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.3")

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

  CPMAddPackage(
    NAME fmt GIT_TAG 10.2.1 GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" # create an installable target
  )

  CPMAddPackage(
    GITHUB_REPOSITORY jarro2783/cxxopts VERSION 3.0.0
    OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO"
    "CXXOPTS_GREETER_INSTALL YES")

  option(GREETER_COVERAGE "Enable test coverage" OFF)

  CPMAddPackage("gh:doctest/doctest@2.4.9")

  file(GLOB sources CONFIGURE_DEPENDS "./test/*.cpp")
  add_executable(GreeterTests ${sources})
  target_link_libraries(GreeterTests doctest::doctest Greeter)
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
endif()

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
