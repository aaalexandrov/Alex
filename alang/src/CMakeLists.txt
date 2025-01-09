set(TARGET_NAME alang)

set(SOURCES 
	"alang.cpp" 
	"alang.h" 
	"common.h" 
	"common.cpp"
	"compile.h"
	"compile.cpp"
	"core.h" 
	"core.cpp" 
	"dbg.h" 
	"dbg.cpp" 
	"defs.h" 
	"defs.cpp"
	"error.h"
	"error.cpp"
	"parse.h" 
	"parse.cpp" 
	"rtti.h" 
	"rtti.cpp"
	"token.h" 
	"token.cpp" 
	"utf8.h" 
	"utf8.cpp" 
)

set(LIBRARIES 
	)	

add_executable(${TARGET_NAME} ${SOURCES})

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

target_link_libraries(${TARGET_NAME} ${LIBRARIES})

