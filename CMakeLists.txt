cmake_minimum_required(VERSION 3.14...3.22)

project(Greeter VERSION 1.0 LANGUAGES CXX)

# CPM
set(CPM_USE_NAMED_CACHE_DIRECTORIES ON)
set(CPM_SOURCE_CACHE "$ENV{HOME}/.cache/CPM")
include(cmake/CPM.cmake)


set(USE_SANITIZER OFF CACHE STRING "What sanitizers to use")
if(USE_SANITIZER)
  message("USE_SANITIZER is '${USE_SANITIZER}'")
  CPMAddPackage(
    "gh:StableCoder/cmake-scripts#1f822d1fc87c8d7720c074cde8a278b44963c354")
  # There are more tools here, but let's just use the sanitizer
  include(${cmake-scripts_SOURCE_DIR}/sanitizers.cmake)
endif()

set(USE_CCACHE ON CACHE BOOL "Whether to use CCache")
if(USE_CCACHE)
  message("USE_CCACHE is '${USE_CCACHE}'")
  CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.3")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GREETER_DEV OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(GREETER_DEV ON)
endif()

option(GREETER_STANDALONE "Generate 'GreeterExec' target" ${GREETER_DEV})
option(GREETER_TESTS "Generate 'GreeterTests' target" ${GREETER_DEV})
option(GREETER_DOCS "Generate 'GreeterDocs' target" ${GREETER_DEV})
option(GREETER_LIB "Generate main 'Greeter' target" ON)
option(GREETER_INSTALL "Generate 'install' target" ON)
option(GREETER_FORMATTER
       "Generate 'format', 'check-format' and 'fix-format' targets"
       ${GREETER_DEV})

# Main library
if(GREETER_LIB)
  CPMAddPackage(
    NAME fmt GIT_TAG 9.1.0 GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" # create an installable target
  )

  file(GLOB_RECURSE headers CONFIGURE_DEPENDS "./include/*.h")
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS "./src/libgreeter/*.cpp")
  add_library(${PROJECT_NAME} ${headers} ${sources})
  set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
  target_compile_options(
    ${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")
  target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

  target_include_directories(
    ${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
           $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>)
endif()

# Executable
if(GREETER_STANDALONE)
  # assert(GREETER_LIB "Can't configure executable with GREETER_LIB=OFF!")
  CPMAddPackage(
    GITHUB_REPOSITORY jarro2783/cxxopts VERSION 3.0.0
    OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO"
            "CXXOPTS_GREETER_INSTALL YES")

  file(GLOB sources CONFIGURE_DEPENDS
       ${CMAKE_CURRENT_SOURCE_DIR}/src/greeter/*.cpp)
  add_executable(GreeterExec ${sources})
  set_target_properties(GreeterExec PROPERTIES CXX_STANDARD 17 OUTPUT_NAME
                                                               "Greeter")
  target_link_libraries(GreeterExec ${PROJECT_NAME} cxxopts)
endif()

# Self-tests
if(GREETER_TESTS)
  # assert(GREETER_LIB "Can't configure tests with GREETER_LIB=OFF!")
  option(GREETER_COVERAGE "Enable test coverage" OFF)

  CPMAddPackage("gh:doctest/doctest@2.4.9")

  file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
  add_executable(GreeterTests ${sources})
  target_link_libraries(GreeterTests doctest::doctest ${PROJECT_NAME})
  set_target_properties(GreeterTests PROPERTIES CXX_STANDARD 17)

  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES
                                              "GNU")
    target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wpedantic -Wextra
                                                  -Werror)
  elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PUBLIC /W4 /WX)
    target_compile_definitions(${PROJECT_NAME}
                               PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
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

# Documentation
if(GREETER_DOCS)
  CPMAddPackage("gh:mosra/m.css#a0d292ec311b97fefd21e93cdefb60f88d19ede6")

  set(DOXYGEN_PROJECT_NAME Greeter)
  set(DOXYGEN_PROJECT_VERSION ${Greeter_VERSION})
  set(DOXYGEN_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
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
endif()

if(GREETER_FORMATTER)
  CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")
endif()

# Installable target
if(GREETER_INSTALL)
  # assert(GREETER_LIB "Can't configure install with GREETER_LIB=OFF!")
  CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")
  string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)
  packageProject(
    NAME ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    NAMESPACE ${PROJECT_NAME}
    BINARY_DIR ${PROJECT_BINARY_DIR}
    INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
    INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
    VERSION_HEADER "${VERSION_HEADER_LOCATION}"
    COMPATIBILITY SameMajorVersion
    DEPENDENCIES "fmt 9.1.0")
endif()
