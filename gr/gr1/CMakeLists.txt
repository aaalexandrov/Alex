
set(gr_implementation VK CACHE STRING "gr graphics implementation")
set_property(CACHE gr_implementation PROPERTY STRINGS VK STUB)

set(dir_SOURCES
	buffer.h
	buffer.cpp

	definitions.h
	definitions.cpp

	device.h
	device.cpp

	execution_queue.h
	execution_queue.cpp

	graphics_exception.h
	graphics_exception.cpp
	
	host.h
	host.cpp
	
	image.h
	image.cpp
	
	output_pass.h
	output_pass.cpp
	
	presentation_surface.h
	presentation_surface.cpp
	
	render_pass.h
	render_pass.cpp
	
	render_state.h
	render_state.cpp
	
	resource.h
	resource.cpp
	
	rttr_factory.h
	rttr_factory.cpp

	sampler.h
	sampler.cpp

	shader.h
	shader.cpp

	utl/gltf.h
	utl/gltf.cpp
)

if (WIN32)
	list(APPEND dir_SOURCES 
		win32/presentation_surface_create_data_win32.h
		win32/presentation_surface_create_data_win32.cpp
	)
endif()

if(gr_implementation STREQUAL VK)
	list(APPEND dir_SOURCES
		vk/buffer_copy_pass_vk.h
		vk/buffer_copy_pass_vk.cpp

		vk/buffer_transition_pass_vk.h
		vk/buffer_transition_pass_vk.cpp

		vk/buffer_vk.h
		vk/buffer_vk.cpp
	
		vk/descriptor_set_store.h
		vk/descriptor_set_store.cpp

		vk/device_vk.h
		vk/device_vk.cpp 
	
		vk/execution_queue_vk.h
		vk/execution_queue_vk.cpp

		vk/final_pass_vk.h
		vk/final_pass_vk.cpp

		vk/host_allocation_tracker_vk.h
		vk/host_allocation_tracker_vk.cpp

		vk/image_buffer_copy_pass_vk.h
		vk/image_buffer_copy_pass_vk.cpp

		vk/image_transition_pass_vk.h
		vk/image_transition_pass_vk.cpp

		vk/image_vk.h
		vk/image_vk.cpp
		
		vk/output_pass_vk.h
		vk/output_pass_vk.cpp

		vk/pipeline_store.h
		vk/pipeline_store.cpp

		vk/present_pass_vk.h
		vk/present_pass_vk.cpp

		vk/presentation_surface_vk.h
		vk/presentation_surface_vk.cpp

		vk/queue_vk.h
		vk/queue_vk.cpp

		vk/render_pass_vk.h
		vk/render_pass_vk.cpp

		vk/render_state_vk.h
		vk/render_state_vk.cpp

		vk/sampler_vk.h
		vk/sampler_vk.cpp

		vk/shader_vk.h
		vk/shader_vk.cpp

		vk/vk.h
		
		vk/vma.h
		vk/vma.cpp
	)

	set(third_party_LIBS
		vma
		stb
		rttr_core_lib
		shaderc_shared
		spirv-cross-glsl
		spirv-cross-reflect
		spirv-cross-core
	)

	set_target_properties(shaderc_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

	target_include_directories(${BINARY} PRIVATE $ENV{VK_SDK_PATH}/Include)

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

