
set(gr_implementation VK CACHE STRING "gr graphics implementation")
set_property(CACHE gr_implementation PROPERTY STRINGS VK STUB)

set(dir_SOURCES
	graphics_exception.h
	graphics_exception.cpp

	graphics.h
	graphics.cpp

	presentation_surface.h
	presentation_surface.cpp
)

if (WIN32)
	list(APPEND dir_SOURCES 
		win32/presentation_surface_create_data_win32.h
		win32/presentation_surface_create_data_win32.cpp
	)
endif()

if(gr_implementation STREQUAL VK)
	list(APPEND dir_SOURCES
		vk/vk.h

		vk/host_allocation_tracker_vk.h
		vk/host_allocation_tracker_vk.cpp

		vk/graphics_vk.h
		vk/graphics_vk.cpp

		vk/physical_device_vk.h
		vk/physical_device_vk.cpp

		vk/device_vk.h
		vk/device_vk.cpp

		vk/presentation_surface_vk.h
		vk/presentation_surface_vk.cpp
	)

	target_include_directories(${BINARY} PRIVATE $ENV{VK_SDK_PATH}/include)

	set(vk_LIBS
		vulkan-1
		VkLayer_utils
	)

	list(TRANSFORM vk_LIBS PREPEND $ENV{VK_SDK_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX})
	list(TRANSFORM vk_LIBS APPEND ${CMAKE_STATIC_LIBRARY_SUFFIX})

	target_link_libraries(${BINARY} PRIVATE ${vk_LIBS})
elseif(gr_implementation STREQUAL STUB)
endif()

target_sources(${BINARY} PRIVATE ${dir_SOURCES} CMakeLists.txt)

