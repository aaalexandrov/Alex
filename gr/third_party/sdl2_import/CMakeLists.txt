
set(BINARY sdl2_import)
add_library(${BINARY} STATIC sdl2_import.cpp)

set(SDL_libs SDL2 SDL2main)

if (WIN32)
    list(TRANSFORM SDL_libs PREPEND "${CMAKE_STATIC_LIBRARY_PREFIX}")
    list(TRANSFORM SDL_libs APPEND ${CMAKE_STATIC_LIBRARY_SUFFIX})

    set(SDL_path $ENV{VK_SDK_PATH})
    list(TRANSFORM SDL_libs PREPEND ${SDL_path}/lib/)
endif()

target_link_libraries(${BINARY} INTERFACE ${SDL_libs})


if (WIN32)
    target_include_directories(${BINARY} INTERFACE ${SDL_path}/include)

    set(SDL_dest_dll ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/SDL2.dll)
    set(SDL_src_dll ${SDL_path}/Bin/SDL2.dll)

    # copy SDL2.dll to output directory on building the sdl2_import project
    add_custom_command(TARGET ${BINARY}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL_src_dll}
        ${SDL_dest_dll})

    #[[
    add_custom_command(OUTPUT ${SDL_dest_dll}
        MAIN_DEPENDENCY ${BINARY}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL_src_dll}
        ${SDL_dest_dll})
    ]]
else()

endif()