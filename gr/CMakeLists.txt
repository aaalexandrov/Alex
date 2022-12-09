cmake_minimum_required(VERSION 3.13)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_MODULE_PATH})
include(IdeGrouping.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

cmake_policy(SET CMP0091 NEW) #to enable the MSVC_RUNTIME_LIBRARY property
project(gr)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(LINUX TRUE)
endif()

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

target_include_directories(${BINARY} PRIVATE .)

target_link_libraries(${BINARY} PRIVATE glm_static tinygltf sdl2_import)

if(${LINUX})
	target_link_libraries(${BINARY} PRIVATE pthread X11)
	if(NOT ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
		target_link_libraries(${BINARY} PRIVATE tbb)
	endif()
endif()

add_subdirectory(util)
#add_subdirectory(platform)
add_subdirectory(gf)
add_subdirectory(gr1)

add_compile_definitions(GR_${gr_implementation})

target_precompile_headers(${BINARY} PRIVATE stdafx.h)

get_target_property(binary_SOURCES ${BINARY} SOURCES)

set_ide_grouping(binary_SOURCES)



