cmake_minimum_required (VERSION 3.8)

set(TARGET_NAME alang)

set(SOURCES 
	"alang.cpp" 
	"alang.h" 
	"analyze.h"
	"analyze.cpp"
	"common.h" 
	"common.cpp"
	"compile.h"
	"compile.cpp"
	"core.h" 
	"core.cpp" 
	"dbg.h" 
	"dbg.cpp" 
	"definitions.h"
	"definitions.cpp"
	"error.h"
	"error.cpp"
	"func.h" 
	"func.cpp" 
	"module.h" 
	"module.cpp" 
	"parse.h" 
	"parse.cpp" 
	"rtti.h" 
	"rtti.cpp"
	"token.h" 
	"token.cpp" 
	"types.h" 
	"types.cpp" 
	"utf8.h" 
	"utf8.cpp" 
	"var.h" 
	"var.cpp" 
	)

set(LIBRARIES 
	)	

add_executable(${TARGET_NAME} ${SOURCES})

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

target_link_libraries(${TARGET_NAME} ${LIBRARIES})

