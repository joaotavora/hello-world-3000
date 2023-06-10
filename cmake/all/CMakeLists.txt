# This CMakeLists.txt allows selecting this directory as a source directory and
# adds all subprojects to a single build.
#
# According to the original "ModernCppStarter" template this allows IDEs
# understand the full project structure.
#
# In our case it allows the top-level Makefile to conveniently configure all the
# whole project's targets.  I decided a separate top-level "all" sub-directory
# was too confusing, so I stashed it here.

cmake_minimum_required(VERSION 3.14...3.22)

project(BuildAll LANGUAGES CXX)

include(../tools.cmake)

# needed to generate test target
enable_testing()

add_subdirectory(
  ${CMAKE_CURRENT_LIST_DIR}/../../greeter ${CMAKE_BINARY_DIR}/greeter
)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../test ${CMAKE_BINARY_DIR}/test)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../doc ${CMAKE_BINARY_DIR}/doc)
