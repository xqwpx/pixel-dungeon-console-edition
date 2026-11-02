# PS3DK wrapper.  The actual compiler setup stays maintained by PS3DK.
if(NOT DEFINED ENV{PS3DK})
    message(FATAL_ERROR "PS3DK is not defined. Set it to the PS3DK installation root.")
endif()

include("$ENV{PS3DK}/cmake/ps3-ppu-toolchain.cmake")
