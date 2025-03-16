# MinGW toolchain file for CMake

# System name
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

# Specify the MinGW compiler
set(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
set(CMAKE_RC_COMPILER i686-w64-mingw32-windres)

# Where to look for the target environment
set(CMAKE_FIND_ROOT_PATH /usr/i686-w64-mingw32)

# Adjust the default behavior of find_XXX() commands
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Don't try to use dll directly
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

# Set optimization flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -mms-bitfields -s")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -mms-bitfields -std=c++11")

# Link flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -Wl,--subsystem,windows,--kill-at,--enable-stdcall-fixup") 