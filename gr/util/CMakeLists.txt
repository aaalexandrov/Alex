
set(dir_SOURCES
	dbg.h
	dbg.cpp

	enumutl.h
	enumutl.cpp

	file.h
	file.cpp

	layout.h
	layout.cpp

	mathutl.h
	mathutl.cpp

	mem.h
	mem.cpp

	multithread.h
	multithread.cpp

	namespace.h

	rect.h
	rect.cpp

	rtti.h
	rtti.cpp
	
	str.h
	str.cpp

	time.h
	time.cpp

	type_factory.h
	type_factory.cpp

	type_layout.h
	type_layout.cpp
	
	utl.h
	utl.cpp
)

target_sources(${BINARY} PRIVATE ${dir_SOURCES} CMakeLists.txt)