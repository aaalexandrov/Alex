
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
	
	render_commands.h
	render_commands.cpp

	render_pass.h
	render_pass.cpp
	
	render_pipeline.h
	render_pipeline.cpp

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

	utl/buffer_content.h
	utl/buffer_content.cpp
	
	utl/font.h
	utl/font.cpp

	utl/gltf.h
	utl/gltf.cpp

	utl/shader_param_data.h
	utl/shader_param_data.cpp
)

if (WIN32)
	list(APPEND dir_SOURCES 
		win32/presentation_surface_create_data_win32.h
		win32/presentation_surface_create_data_win32.cpp
	)
endif()

if (${LINUX})
	list(APPEND dir_SOURCES 
		x11/presentation_surface_create_data_xlib.h
		x11/presentation_surface_create_data_xlib.cpp
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

		vk/render_commands_vk.h
		vk/render_commands_vk.cpp

		vk/render_pass_vk.h
		vk/render_pass_vk.cpp

		vk/render_pipeline_transition_pass_vk.h
		vk/render_pipeline_transition_pass_vk.cpp

		vk/render_pipeline_vk.h
		vk/render_pipeline_vk.cpp

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
		taskflow
		vma
		stb
		rttr_core_lib
	)

	if (WIN32)
		set(VK_SDK_PATH $ENV{VK_SDK_PATH})
	else()
		set(VK_SDK_PATH $ENV{VULKAN_SDK})
	endif()

	target_include_directories(${BINARY} PRIVATE ${VK_SDK_PATH}/include)

	set(vk_LIBS_static
		spirv-cross-glsl
		spirv-cross-reflect
		spirv-cross-core
	)

	set(vk_LIBS_shared
	)

	if (WIN32)
		set(vk_LIBS_static ${vk_LIBS_static} 
			vulkan-1 
			shaderc_shared
		)
	else()
		set(vk_LIBS_shared ${vk_LIBS_shared} 
			vulkan 		
			shaderc_shared
		)
	endif()

	list(TRANSFORM vk_LIBS_static PREPEND ${VK_SDK_PATH}/lib/${CMAKE_STATIC_LIBRARY_PREFIX})
	list(TRANSFORM vk_LIBS_static APPEND ${CMAKE_STATIC_LIBRARY_SUFFIX})

	list(TRANSFORM vk_LIBS_shared PREPEND ${VK_SDK_PATH}/lib/${CMAKE_SHARED_LIBRARY_PREFIX})
	list(TRANSFORM vk_LIBS_shared APPEND ${CMAKE_SHARED_LIBRARY_SUFFIX})

	target_link_libraries(${BINARY} PRIVATE ${vk_LIBS_static} ${vk_LIBS_shared} ${third_party_LIBS})
elseif(gr_implementation STREQUAL STUB)
endif()

target_sources(${BINARY} PRIVATE ${dir_SOURCES} CMakeLists.txt)

