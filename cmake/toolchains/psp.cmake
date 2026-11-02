# PSPSDK toolchain. PSPDEV must point to a PSPSDK installation.
if(NOT DEFINED ENV{PSPDEV})
    message(FATAL_ERROR "PSPDEV is not defined. Set it to the PSPSDK installation root.")
endif()

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR allegrex)
# Resolve through PATH: the Windows PSPDEV binaries are MSYS-hosted, so
# absolute Windows paths are not valid to CMake running from MSYS.
set(CMAKE_C_COMPILER psp-gcc)
set(CMAKE_CXX_COMPILER psp-g++)
set(CMAKE_C_COMPILER_LAUNCHER "${CMAKE_CURRENT_LIST_DIR}/psp-compiler-launcher.cmd")
set(CMAKE_CXX_COMPILER_LAUNCHER "${CMAKE_CURRENT_LIST_DIR}/psp-compiler-launcher.cmd")
set(CMAKE_AR psp-ar)
set(CMAKE_RANLIB psp-ranlib)
set(CMAKE_STRIP psp-strip)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
