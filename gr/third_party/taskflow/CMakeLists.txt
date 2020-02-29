file(GLOB_RECURSE HEADERS *.hpp *.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_library(taskflow STATIC ${HEADERS})
target_include_directories(taskflow INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_ide_grouping(HEADERS)