
set(dir_SOURCES
	dbg.h
	dbg.cpp

	enumutl.h
	enumutl.cpp

	mathutl.h
	mathutl.cpp

	mem.h
	mem.cpp

	multithread.h
	multithread.cpp

	namespace.h

	rect.h
	rect.cpp
	
	time.h
	time.cpp
)

target_sources(${BINARY} PRIVATE ${dir_SOURCES} CMakeLists.txt)