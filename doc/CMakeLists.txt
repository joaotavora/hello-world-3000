set(CPM_USE_NAMED_CACHE_DIRECTORIES ON)
set(CPM_SOURCE_CACHE "$ENV{HOME}/.cache/CPM")
include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

CPMAddPackage("gh:mosra/m.css#a0d292ec311b97fefd21e93cdefb60f88d19ede6")

set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doxygen")
configure_file(./Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
configure_file(./conf.py ${CMAKE_CURRENT_BINARY_DIR}/conf.py)

add_custom_target(
  GreeterDocs
  ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIRECTORY}"
  COMMAND "${m.css_SOURCE_DIR}/documentation/doxygen.py"
  "${CMAKE_CURRENT_BINARY_DIR}/conf.py"
  COMMAND echo "Docs written to: ${DOXYGEN_OUTPUT_DIRECTORY}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
