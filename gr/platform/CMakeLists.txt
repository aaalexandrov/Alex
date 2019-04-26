
set(dir_SOURCES
	platform.h
	platform.cpp

	resource.h
	resource.cpp
	
	window.h
	window.cpp

	input.h
	input.cpp
)

if (WIN32)
	list(APPEND dir_SOURCES
		win32/platform_win32.h
		win32/platform_win32.cpp
		
		win32/window_win32.h
		win32/window_win32.cpp
	)
endif()

target_sources(${BINARY} PRIVATE ${dir_SOURCES} CMakeLists.txt)