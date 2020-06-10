cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})
include(PrecompiledHeader.cmake)
include(IdeGrouping.cmake)

set(CMAKE_CXX_STANDARD 17)

cmake_policy(SET CMP0091 NEW) #to enable the MSVC_RUNTIME_LIBRARY property
project(gr)

add_compile_definitions(UNICODE _UNICODE)

add_subdirectory(third_party)

set(SOURCES
	stdafx.h
	stdafx.cpp
	
	grtest.cpp
)

set(BINARY grtest)

add_executable(${BINARY} ${SOURCES})
set_property(TARGET ${BINARY} PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)
add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)

target_link_libraries(${BINARY} PRIVATE glm_static tinygltf)

target_include_directories(${BINARY} PRIVATE .)

add_subdirectory(util)
add_subdirectory(platform)
add_subdirectory(gf)
add_subdirectory(gr1)

add_compile_definitions(GR_${gr_implementation})

add_precompiled_header(${BINARY} stdafx.h FORCEINCLUDE SOURCE_CXX stdafx.cpp)

get_target_property(binary_SOURCES ${BINARY} SOURCES)

set_ide_grouping(binary_SOURCES)



