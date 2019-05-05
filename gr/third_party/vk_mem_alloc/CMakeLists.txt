
set(BINARY vk_mem_alloc)

set(SOURCES
	vk_mem_alloc.h
	vk_mem_alloc.natvis
)

add_library(${BINARY} STATIC ${SOURCES})
set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${BINARY} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)


set_ide_grouping(SOURCES)