
set(BINARY stb)

set(SOURCES
	stb_image.h
	stb_image_resize.h
	stb_image_write.h

	stb.cpp
)

add_library(${BINARY} STATIC ${SOURCES})
set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${BINARY} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


set_ide_grouping(SOURCES)