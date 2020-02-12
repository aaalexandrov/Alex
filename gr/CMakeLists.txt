cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})
include(PrecompiledHeader.cmake)
include(IdeGrouping.cmake)

set(CMAKE_CXX_STANDARD 11)

project(gr)

add_definitions(-DUNICODE -D_UNICODE)

add_subdirectory(third_party)

set(SOURCES
	stdafx.h
	stdafx.cpp
	
	grtest.cpp
)

set(BINARY grtest)

add_executable(${BINARY} ${SOURCES})

target_link_libraries(${BINARY} PRIVATE glm_static)

target_include_directories(${BINARY} PRIVATE .)

add_subdirectory(util)
add_subdirectory(platform)
add_subdirectory(gr1)

add_definitions(-DGR_${gr_implementation})

add_precompiled_header(${BINARY} stdafx.h FORCEINCLUDE SOURCE_CXX stdafx.cpp)

get_target_property(binary_SOURCES ${BINARY} SOURCES)

set_ide_grouping(binary_SOURCES)



