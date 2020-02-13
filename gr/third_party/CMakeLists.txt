
add_subdirectory(glm)
set_target_properties(glm_shared glm_static PROPERTIES 
	INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}" 
	FOLDER glm
)

add_subdirectory(vma)
add_subdirectory(stb)
add_subdirectory(tinygltf)

set(BUILD_RTTR_DYNAMIC OFF)
set(BUILD_STATIC ON)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_EXAMPLES OFF)
set(BUILD_INSTALLER OFF)
set(BUILD_PACKAGE OFF)

add_subdirectory(rttr)

set(SHADERC_SKIP_INSTALL ON)
set(SHADERC_SKIP_TESTS ON)
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(shaderc)

set(SPIRV_CROSS_CLI OFF CACHE BOOL "Build the CLI binary. Requires SPIRV_CROSS_STATIC.")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "Enable SPIRV-Cross tests.")
set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "Enable MSL target support.")
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "Enable C++ target support.")
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "Enable C API wrapper support in static library.")
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "Enable util module support.")

add_subdirectory(SPIRV-Cross)