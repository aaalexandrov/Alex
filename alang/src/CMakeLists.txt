cmake_minimum_required (VERSION 3.8)

set(TARGET_NAME alang)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include(AddLLVM)

set(LLVM_LINK_COMPONENTS
  Core
  ExecutionEngine
  Interpreter
  MC
  MCJIT
  Support
  nativecodegen
  )

set(SOURCES 
	"alang.cpp" 
	"alang.h" 
	"analyze.h"
	"analyze.cpp"
	"token.h" 
	"token.cpp" 
	"utf8.h" 
	"utf8.cpp" 
	"dbg.h" 
	"dbg.cpp" 
	"parse.h" 
	"parse.cpp" 
	"types.h" 
	"types.cpp" 
	"common.h" 
	"module.h" 
	"module.cpp" 
	"core.h" 
	"core.cpp" 
	"func.h" 
	"func.cpp" 
	"var.h" 
	"var.cpp" 
	"rtti.h" 
	"rtti.cpp"
	)

set(LIBRARIES 
	)	

add_executable(${TARGET_NAME} ${SOURCES})

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

if(MSVC)
	# We're linking to LLVM release libs, so set options that won't conflict
	target_compile_definitions(${TARGET_NAME} PRIVATE _ITERATOR_DEBUG_LEVEL=0)
	target_compile_options(${TARGET_NAME} PRIVATE /MD)
endif()

target_link_libraries(${TARGET_NAME} ${LIBRARIES})

