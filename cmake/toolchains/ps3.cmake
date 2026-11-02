# PS3 homebrew toolchain supplied by ps3dev/PSL1GHT in WSL.
if(NOT DEFINED ENV{PS3DEV})
    message(FATAL_ERROR "Set PS3DEV to the ps3dev root, for example /usr/local/ps3dev.")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR powerpc64)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(PS3DEV "$ENV{PS3DEV}")
set(CMAKE_C_COMPILER "${PS3DEV}/ppu/bin/ppu-gcc")
set(CMAKE_CXX_COMPILER "${PS3DEV}/ppu/bin/ppu-g++")
set(CMAKE_AR "${PS3DEV}/ppu/bin/ppu-ar")
set(CMAKE_RANLIB "${PS3DEV}/ppu/bin/ppu-ranlib")
set(CMAKE_STRIP "${PS3DEV}/ppu/bin/ppu-strip")

set(CMAKE_FIND_ROOT_PATH "${PS3DEV}/ppu" "${PS3DEV}/portlibs/ppu")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
