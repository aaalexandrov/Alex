# CMakeList.txt : CMake project for alang, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (alang "alang.cpp" "alang.h" "token.h" "token.cpp" "utf8.h" "utf8.cpp" "dbg.h" "dbg.cpp" "parse.h" "parse.cpp")

# TODO: Add tests and install targets if needed.
