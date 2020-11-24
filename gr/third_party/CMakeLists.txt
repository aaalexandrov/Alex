
set(BUILD_STATIC_LIBS ON)
set(GLM_DIR glm)
add_subdirectory(${GLM_DIR} ${PROJECT_BINARY_DIR}/glm)
set_target_properties(glm_static PROPERTIES 
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}" 
	FOLDER ${GLM_DIR}
)

add_subdirectory(taskflow)

add_subdirectory(vma)
set_property(TARGET vma APPEND PROPERTY COMPILE_DEFINITIONS "_ITERATOR_DEBUG_LEVEL=0")
set_property(TARGET vma PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

add_subdirectory(stb)
add_subdirectory(sdl2_import)

add_subdirectory(tinygltf)
set_property(TARGET tinygltf APPEND PROPERTY COMPILE_DEFINITIONS "_ITERATOR_DEBUG_LEVEL=0")
set_property(TARGET tinygltf PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

set(BUILD_RTTR_DYNAMIC OFF)
set(BUILD_STATIC ON)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_INSTALLER OFF)
set(BUILD_PACKAGE OFF)

add_subdirectory(rttr)
set_property(TARGET rttr_core_lib APPEND PROPERTY COMPILE_DEFINITIONS "_ITERATOR_DEBUG_LEVEL=0")
set_property(TARGET rttr_core_lib PROPERTY MSVC_RUNTIME_LIBRARY MultiThreadedDLL)

