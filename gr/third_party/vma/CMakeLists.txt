
set(BINARY vma)

set(SOURCES
	vk_mem_alloc.h
	vk_mem_alloc.natvis

	vk_mem_alloc.cpp
)

add_library(${BINARY} STATIC ${SOURCES})
set_target_properties(${BINARY} PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(${BINARY} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

if (WIN32)
	set(VK_SDK_PATH $ENV{VK_SDK_PATH})
else()
	set(VK_SDK_PATH $ENV{VULKAN_SDK})
endif()

target_include_directories(${BINARY} PRIVATE ${VK_SDK_PATH}/include)


set_ide_grouping(SOURCES)