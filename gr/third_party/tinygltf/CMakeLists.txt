set(BINARY tinygltf)

set(SOURCES
	json.hpp
	tiny_gltf.h

	tiny_gltf.cpp
)

add_library(${BINARY} STATIC ${SOURCES})
set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${BINARY} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/.. PRIVATE ../stb)

set_ide_grouping(SOURCES)