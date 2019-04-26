
set(dir_SOURCES
	dbg.h
	dbg.cpp

	mem.h
	mem.cpp

	enumutl.h
	enumutl.cpp

	time.h
	time.cpp

	mathutl.h
	mathutl.cpp

	rect.h
	rect.cpp
)

target_sources(${BINARY} PRIVATE ${dir_SOURCES} CMakeLists.txt)