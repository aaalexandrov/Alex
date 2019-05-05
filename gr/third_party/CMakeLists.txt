
add_subdirectory(glm)
set_target_properties(glm_shared glm_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(vk_mem_alloc)