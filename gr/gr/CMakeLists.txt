
set(gr_implementation VK CACHE STRING "gr graphics implementation")
set_property(CACHE gr_implementation PROPERTY STRINGS VK STUB)

set(dir_SOURCES
	graphics.h
	graphics.cpp

	graphics_exception.h
	graphics_exception.cpp

	image.h
	image.cpp

	presentation_surface.h
	presentation_surface.cpp

	shader.h
	shader.cpp
)

if (WIN32)
	list(APPEND dir_SOURCES 
		win32/presentation_surface_create_data_win32.h
		win32/presentation_surface_create_data_win32.cpp
	)
endif()

if(gr_implementation STREQUAL VK)
	list(APPEND dir_SOURCES

		vk/device_vk.h
		vk/device_vk.cpp

		vk/graphics_vk.h
		vk/graphics_vk.cpp

		vk/host_allocation_tracker_vk.h
		vk/host_allocation_tracker_vk.cpp

		vk/image_vk.h
		vk/image_vk.cpp

		vk/presentation_surface_vk.h
		vk/presentation_surface_vk.cpp

		vk/physical_device_vk.h
		vk/physical_device_vk.cpp

		vk/queue_vk.h
		vk/queue_vk.cpp

		vk/shader_vk.h
		vk/shader_vk.cpp

		vk/vk.h
		
		vk/vma.h
		vk/vma.cpp
	)

	set(third_party_LIBS
		vma
		shaderc_shared
		spirv-cross-glsl
		spirv-cross-reflect
		spirv-cross-core
	)

	set_target_properties(shaderc_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

	target_include_directories(${BINARY} PRIVATE $ENV{VK_SDK_PATH}/include)

	set(vk_LIBS
		vulkan-1
		VkLayer_utils
	)

	list(TRANSFORM vk_LIBS PREPEND $ENV{VK_SDK_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX})
	list(TRANSFORM vk_LIBS APPEND ${CMAKE_STATIC_LIBRARY_SUFFIX})

	target_link_libraries(${BINARY} PRIVATE ${vk_LIBS} ${third_party_LIBS})
elseif(gr_implementation STREQUAL STUB)
endif()

target_sources(${BINARY} PRIVATE ${dir_SOURCES} CMakeLists.txt)

