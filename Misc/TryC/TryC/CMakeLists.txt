# CMakeList.txt : CMake project for TryC, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.21)


# Add source to this project's executable.
add_executable (TryC "TryC.c"  "vec.h" "hash.h" "hash.c" "vec.c")
set_property(TARGET TryC PROPERTY C_STANDARD 23)

# TODO: Add tests and install targets if needed.
