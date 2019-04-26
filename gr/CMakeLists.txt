cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})
include(PrecompiledHeader.cmake)

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
add_subdirectory(gr)

add_definitions(-DGR_${gr_implementation})

add_precompiled_header(${BINARY} stdafx.h FORCEINCLUDE)

get_target_property(binary_SOURCES ${BINARY} SOURCES)

# set IDE grouping for all sources
foreach(FILE ${binary_SOURCES}) 
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")

    source_group("${GROUP}" FILES "${FILE}")
endforeach()



