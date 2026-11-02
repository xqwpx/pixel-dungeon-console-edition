function(pd_configure_platform)
    string(TOLOWER "${PD_PLATFORM}" PD_PLATFORM)
    set(PD_PLATFORM "${PD_PLATFORM}" CACHE STRING "PIXEL DUNGEON CONSOLE EDITION target platform" FORCE)
    if(PD_PLATFORM STREQUAL "windows")
        add_compile_definitions(PD_PLATFORM_WINDOWS=1 PD_ASSET_ROOT="assets/" PD_USERDATA_ROOT="userdata/")
    elseif(PD_PLATFORM STREQUAL "psvita")
        add_compile_definitions(PD_PLATFORM_PSVITA=1 PD_ASSET_ROOT="ux0:data/PixelDungeonVita/assets/" PD_USERDATA_ROOT="ux0:data/PixelDungeonVita/")
    elseif(PD_PLATFORM STREQUAL "psp")
        # The PSP loader starts EBOOT.PBP in its game directory.  Relative
        # paths keep the package movable and avoid Cygwin's ms0: path rewrite
        # while compiling the Windows-hosted PSPSDK toolchain.
        add_compile_definitions(PD_PLATFORM_PSP=1)
    elseif(PD_PLATFORM STREQUAL "ps3")
        add_compile_definitions(PD_PLATFORM_PS3=1 PD_ASSET_ROOT="/dev_hdd0/game/PDXCE0001/USRDIR/assets/" PD_USERDATA_ROOT="/dev_hdd0/game/PDXCE0001/USRDIR/userdata/")
    elseif(PD_PLATFORM STREQUAL "ps2")
        # The first PS2 target is run from a PS2SDK host filesystem in the
        # emulator.  Packaging for mass:/ and disc images is added separately.
        add_compile_definitions(PD_PLATFORM_PS2=1 PD_ASSET_ROOT="host:assets/" PD_USERDATA_ROOT="host:userdata/")
    elseif(PD_PLATFORM STREQUAL "ps1")
        add_compile_definitions(PD_PLATFORM_PS1=1)
    endif()
endfunction()
