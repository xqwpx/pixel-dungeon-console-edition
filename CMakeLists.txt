cmake_minimum_required(VERSION 3.16)
project(PixelDungeonConsoleEdition LANGUAGES C CXX ASM)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(PixelDungeonOptions)
include(PixelDungeonPlatform)
include(PixelDungeonCompiler)

pd_configure_platform()

if(PD_PLATFORM STREQUAL "ps1")
    # libpsn00b exports -fno-rtti as an interface option.  The shared game
    # model deliberately uses typeid/dynamic_cast, so remove only that one
    # PS1 default while retaining the SDK's freestanding/no-exception flags.
    get_target_property(PD_PS1_SDK_OPTIONS psn00bsdk INTERFACE_COMPILE_OPTIONS)
    list(REMOVE_ITEM PD_PS1_SDK_OPTIONS "-fno-rtti")
    set_property(TARGET psn00bsdk PROPERTY INTERFACE_COMPILE_OPTIONS "${PD_PS1_SDK_OPTIONS}")

    # The initial PS1 target is deliberately isolated in the platform folder.
    # Shared gameplay code is brought in only after the PS1 renderer/asset
    # adapter is complete; no game logic is duplicated here.
    #
    # Keep the expanding real dungeon graph separate from the menu executable
    # until every direct dependency is available.  This lets us compile the
    # common Level implementation with the PSn00bSDK without making the
    # currently working ISO inherit its unresolved gameplay symbols.
    add_library(PixelDungeonConsoleEditionPS1LevelGraph OBJECT
        src/game/levels/Level.cpp
        src/game/levels/Patch.cpp
        src/game/levels/Room.cpp
        src/game/levels/RegularLevel.cpp
        src/game/levels/SewerLevel.cpp
        src/game/levels/painters/Painter.cpp
        src/game/levels/painters/EntrancePainter.cpp
        src/game/levels/painters/ExitPainter.cpp
        src/game/levels/painters/BossExitPainter.cpp
        src/game/levels/painters/SpecialRoomPainter.cpp
        src/game/levels/painters/StandardPainter.cpp
        src/game/levels/painters/TunnelPainter.cpp
        src/game/levels/painters/PassagePainter.cpp
        src/game/actors/mobs/Mob.cpp
        src/game/actors/mobs/Bestiary.cpp
        src/game/actors/mobs/Rat.cpp
        src/game/actors/mobs/Gnoll.cpp
        src/game/actors/mobs/Crab.cpp
        src/game/actors/mobs/Goo.cpp
        src/game/actors/mobs/Piranha.cpp
        src/game/actors/mobs/SpecialMob.cpp
        src/game/actors/mobs/Swarm.cpp
        src/game/actors/mobs/Skeleton.cpp
        src/game/actors/mobs/npcs/MirrorImage.cpp
        src/game/actors/mobs/npcs/DungeonNPC.cpp
        src/game/actors/Actor.cpp
        src/game/actors/Char.cpp
        src/game/actors/Hero.cpp
        src/game/actors/blobs/Blob.cpp
        src/game/actors/blobs/Fire.cpp
        src/game/actors/blobs/ToxicGas.cpp
        src/game/actors/blobs/ParalyticGas.cpp
        src/game/actors/blobs/ConfusionGas.cpp
        src/game/actors/blobs/WellWater.cpp
        src/game/actors/blobs/Alchemy.cpp
        src/game/actors/blobs/Web.cpp
        src/game/actors/blobs/Regrowth.cpp
        src/game/actors/blobs/Foliage.cpp
        src/game/actors/blobs/SacrificialFire.cpp
        src/game/actors/buffs/Blindness.cpp
        src/game/actors/buffs/Buff.cpp
        src/game/actors/buffs/Charm.cpp
        src/game/actors/buffs/Cripple.cpp
        src/game/actors/buffs/Awareness.cpp
        src/game/actors/buffs/MindVision.cpp
        src/game/actors/buffs/Shadows.cpp
        src/game/actors/buffs/Barkskin.cpp
        src/game/actors/buffs/HerbalArmor.cpp
        src/game/actors/buffs/HerbalHealing.cpp
        src/game/actors/buffs/Frost.cpp
        src/game/actors/buffs/Poison.cpp
        src/game/actors/buffs/Roots.cpp
        src/game/actors/buffs/Slow.cpp
        src/game/actors/buffs/Speed.cpp
        src/game/actors/buffs/Vertigo.cpp
        src/game/actors/buffs/GasesImmunity.cpp
        src/game/actors/buffs/Paralysis.cpp
        src/game/actors/buffs/Amok.cpp
        src/game/actors/buffs/Bleeding.cpp
        src/game/actors/buffs/Burning.cpp
        src/game/actors/buffs/Combo.cpp
        src/game/actors/buffs/Fury.cpp
        src/game/actors/buffs/Hunger.cpp
        src/game/actors/buffs/Ooze.cpp
        src/game/actors/buffs/Sleep.cpp
        src/game/actors/buffs/Terror.cpp
        src/game/levels/traps/ToxicTrap.cpp
        src/game/levels/traps/FireTrap.cpp
        src/game/levels/traps/ParalyticTrap.cpp
        src/game/levels/traps/PoisonTrap.cpp
        src/game/levels/traps/AlarmTrap.cpp
        src/game/levels/traps/GrippingTrap.cpp
        src/game/levels/traps/LightningTrap.cpp
        src/game/levels/traps/SummoningTrap.cpp
        src/game/items/Heap.cpp
        src/game/items/Item.cpp
        src/game/items/ConsumableCatalog.cpp
        src/game/items/EquipmentCatalog.cpp
        src/game/items/Gold.cpp
        src/game/items/SpecialItems.cpp
        src/game/items/Generator.cpp
        src/game/items/KindOfWeapon.cpp
        src/game/items/armor/Armor.cpp
        src/game/items/armor/ClothArmor.cpp
        src/game/items/bags/Keyring.cpp
        src/game/items/food/Food.cpp
        src/game/items/rings/Ring.cpp
        src/game/items/wands/Wand.cpp
        src/game/items/wands/WandOfMagicMissile.cpp
        src/game/items/weapon/melee/MeleeWeapon.cpp
        src/game/items/weapon/melee/Dagger.cpp
        src/game/items/weapon/melee/Knuckles.cpp
        src/game/items/weapon/melee/ShortSword.cpp
        src/game/items/weapon/missiles/Boomerang.cpp
        src/game/items/weapon/missiles/Dart.cpp
        src/game/mechanics/ShadowCaster.cpp
        src/game/mechanics/Ballistica.cpp
        src/game/utils/Random.cpp
        src/game/Dungeon.cpp
        src/game/Progression.cpp
        src/game/ui/QuickSlot.cpp
        src/game/actors/mobs/Albino.cpp
        src/game/actors/mobs/Thief.cpp
        src/game/actors/mobs/ZoneMob.cpp)
    # The initial list makes the dependency frontier explicit.  Include the
    # remaining game implementation as well when linking it into the PS1
    # executable: generation reaches item catalogues, save/load and actor
    # specialisations through virtual dispatch.
    file(GLOB_RECURSE PD_PS1_GAME_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.cpp")
    list(REMOVE_ITEM PD_PS1_GAME_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/game/ui/InventoryModel.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/game/ui/BuffIndicator.cpp")
    target_sources(PixelDungeonConsoleEditionPS1LevelGraph PRIVATE
        ${PD_PS1_GAME_SOURCES}
        src/platform/ps1/PS1Localization.cpp)
    target_include_directories(PixelDungeonConsoleEditionPS1LevelGraph PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps1/compat"
        "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_compile_features(PixelDungeonConsoleEditionPS1LevelGraph PRIVATE cxx_std_17)
    # PSn00bSDK's toolchain file appends -fno-rtti after target options.
    # COMPILE_FLAGS is emitted last, so the shared typeid/dynamic_cast graph
    # retains RTTI for this isolated object target.
    set_property(TARGET PixelDungeonConsoleEditionPS1LevelGraph APPEND_STRING
        PROPERTY COMPILE_FLAGS " -frtti")

    psn00bsdk_add_executable(PixelDungeonConsoleEditionPS1 GPREL
        src/platform/ps1/main.cpp
        src/platform/ps1/PS1Input.cpp
        src/platform/ps1/PS1MemoryCard.cpp
        src/platform/ps1/PS1CppRuntime.cpp
        src/core/GameSession.cpp
        )
    target_include_directories(PixelDungeonConsoleEditionPS1 PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps1/compat"
        "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_compile_features(PixelDungeonConsoleEditionPS1 PRIVATE cxx_std_17)
    # Shared dungeon code uses typeid/dynamic_cast for actors and buffs.
    # PSn00bSDK defaults to -fno-rtti, so restore RTTI only for this target.
    set_property(TARGET PixelDungeonConsoleEditionPS1 APPEND_STRING
        PROPERTY COMPILE_FLAGS " -frtti")
    target_sources(PixelDungeonConsoleEditionPS1 PRIVATE
        $<TARGET_OBJECTS:PixelDungeonConsoleEditionPS1LevelGraph>)
    target_link_libraries(PixelDungeonConsoleEditionPS1 PRIVATE
        "$ENV{PSN00BSDK}/mipsel-none-elf/lib/soft-float/libsupc++.a")

    set(PD_PS1_BANNERS_TIM "${CMAKE_CURRENT_BINARY_DIR}/banners.tim")
    set(PD_PS1_ARCS1_TIM "${CMAKE_CURRENT_BINARY_DIR}/arcs1.tim")
    set(PD_PS1_ARCS2_TIM "${CMAKE_CURRENT_BINARY_DIR}/arcs2.tim")
    set(PD_PS1_DASHBOARD_TIM "${CMAKE_CURRENT_BINARY_DIR}/dashboard.tim")
    set(PD_PS1_AVATARS_TIM "${CMAKE_CURRENT_BINARY_DIR}/avatars.tim")
    add_custom_command(
        OUTPUT "${PD_PS1_BANNERS_TIM}"
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
                -InputPath "${CMAKE_CURRENT_SOURCE_DIR}/assets/banners.png"
                -OutputPath "${PD_PS1_BANNERS_TIM}"
                -VramX 512 -VramY 0
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/banners.png"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
        COMMENT "Converting PS1 TIM texture banners.png"
        VERBATIM)
    add_custom_command(
        OUTPUT "${PD_PS1_ARCS1_TIM}"
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
                -InputPath "${CMAKE_CURRENT_SOURCE_DIR}/assets/arcs1.png"
                -OutputPath "${PD_PS1_ARCS1_TIM}"
                -VramX 768 -VramY 0
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/arcs1.png"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
        COMMENT "Converting PS1 TIM texture arcs1.png"
        VERBATIM)
    add_custom_command(
        OUTPUT "${PD_PS1_ARCS2_TIM}"
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
                -InputPath "${CMAKE_CURRENT_SOURCE_DIR}/assets/arcs2.png"
                -OutputPath "${PD_PS1_ARCS2_TIM}"
                -VramX 768 -VramY 64
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/arcs2.png"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
        COMMENT "Converting PS1 TIM texture arcs2.png"
        VERBATIM)
    add_custom_command(
        OUTPUT "${PD_PS1_DASHBOARD_TIM}"
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
                -InputPath "${CMAKE_CURRENT_SOURCE_DIR}/assets/dashboard.png"
                -OutputPath "${PD_PS1_DASHBOARD_TIM}"
                -VramX 512 -VramY 256
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/dashboard.png"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
        COMMENT "Converting PS1 TIM texture dashboard.png"
        VERBATIM)
    add_custom_command(
        OUTPUT "${PD_PS1_AVATARS_TIM}"
        COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
                -InputPath "${CMAKE_CURRENT_SOURCE_DIR}/assets/avatars.png"
                -OutputPath "${PD_PS1_AVATARS_TIM}"
                -VramX 640 -VramY 256
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/avatars.png"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/convert_ps1_tim.ps1"
        COMMENT "Converting PS1 TIM texture avatars.png"
        VERBATIM)
    add_custom_target(PixelDungeonConsoleEditionPS1Assets DEPENDS
        "${PD_PS1_BANNERS_TIM}" "${PD_PS1_ARCS1_TIM}"
        "${PD_PS1_ARCS2_TIM}" "${PD_PS1_DASHBOARD_TIM}" "${PD_PS1_AVATARS_TIM}"
        )
    add_dependencies(PixelDungeonConsoleEditionPS1 PixelDungeonConsoleEditionPS1Assets)
    psn00bsdk_target_incbin(PixelDungeonConsoleEditionPS1 PRIVATE ps1_banners "${PD_PS1_BANNERS_TIM}")
    psn00bsdk_target_incbin(PixelDungeonConsoleEditionPS1 PRIVATE ps1_arcs1 "${PD_PS1_ARCS1_TIM}")
    psn00bsdk_target_incbin(PixelDungeonConsoleEditionPS1 PRIVATE ps1_arcs2 "${PD_PS1_ARCS2_TIM}")
    psn00bsdk_target_incbin(PixelDungeonConsoleEditionPS1 PRIVATE ps1_dashboard "${PD_PS1_DASHBOARD_TIM}")
    psn00bsdk_target_incbin(PixelDungeonConsoleEditionPS1 PRIVATE ps1_avatars "${PD_PS1_AVATARS_TIM}")

    psn00bsdk_add_cd_image(
        PixelDungeonConsoleEditionPS1Iso
        PixelDungeonConsoleEditionPS1
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps1/iso.xml"
        DEPENDS PixelDungeonConsoleEditionPS1
                "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps1/system.cnf")
else()
file(GLOB_RECURSE PD_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/game/*.cpp")
list(APPEND PD_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/GameSession.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Localization.cpp")
add_library(pd_core STATIC ${PD_CORE_SOURCES})
target_include_directories(pd_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/include")
pd_apply_compiler_settings(pd_core)

file(GLOB_RECURSE PD_ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/audio/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/render/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/scenes/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ui/*.cpp")
list(APPEND PD_ENGINE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Application.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/OnlineLeaderboard.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/OnlineLeaderboardRequest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/SceneManager.cpp")
if(PD_PLATFORM STREQUAL "ps3")
    list(REMOVE_ITEM PD_ENGINE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/assets/Texture2D.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/render/Renderer2D.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/audio/AudioSystem.cpp")
    list(APPEND PD_ENGINE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps3/PS3Texture2D.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps3/PS3Renderer2D.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps3/PS3Renderer2DAdapter.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps3/PS3AudioSystem.cpp")
elseif(PD_PLATFORM STREQUAL "ps2")
    list(REMOVE_ITEM PD_ENGINE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/audio/AudioSystem.cpp")
    list(APPEND PD_ENGINE_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/ps2/PS2AudioSystem.cpp")
endif()
add_library(pd_engine STATIC ${PD_ENGINE_SOURCES})
target_include_directories(pd_engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/include")
if(PD_PLATFORM STREQUAL "psp")
    file(TO_CMAKE_PATH "$ENV{PSPDEV}" PSPDEV)
    target_include_directories(pd_engine PUBLIC
        "${PSPDEV}/psp/sdk/include"
        "${PSPDEV}/psp/include"
        "${PSPDEV}/psp/include/SDL2"
        "${PSPDEV}/psp/include/freetype2")
endif()
target_link_libraries(pd_engine PUBLIC pd_core)
pd_apply_compiler_settings(pd_engine)

if(PD_PLATFORM STREQUAL "windows")
    enable_language(RC)
    find_package(SDL2 REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    find_package(PNG REQUIRED)
    target_link_libraries(pd_engine PUBLIC SDL2::SDL2 SDL2_ttf::SDL2_ttf SDL2_mixer::SDL2_mixer PNG::PNG winhttp)
    add_library(pd_platform_windows STATIC src/platform/windows/WindowsInput.cpp)
    target_link_libraries(pd_platform_windows PUBLIC SDL2::SDL2)
    target_include_directories(pd_platform_windows PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
    add_executable(PixelDungeonConsoleEdition
        src/platform/windows/main.cpp
        src/platform/windows/pixel_dungeon.rc)
    target_compile_definitions(PixelDungeonConsoleEdition PRIVATE SDL_MAIN_HANDLED)
    target_link_libraries(PixelDungeonConsoleEdition PRIVATE pd_platform_windows pd_engine)
    set_target_properties(PixelDungeonConsoleEdition PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        OUTPUT_NAME "PIXEL DUNGEON CONSOLE EDITION")
    add_custom_target(stage_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/assets" COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/userdata")
    add_dependencies(PixelDungeonConsoleEdition stage_assets)
    # Visual Studio adds the active configuration (for example Debug) to the
    # executable directory.  Keep a runtime copy beside the executable, since
    # the game resolves assets relative to its current working directory.
    add_custom_command(TARGET PixelDungeonConsoleEdition POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:PixelDungeonConsoleEdition>/assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:PixelDungeonConsoleEdition>/userdata")
elseif(PD_PLATFORM STREQUAL "psvita")
    if(NOT DEFINED ENV{VITASDK})
        message(FATAL_ERROR "VITASDK is not defined. Use --platform psvita only in a VitaSDK environment.")
    endif()
    include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
    find_package(SDL2 REQUIRED)
    find_package(PNG REQUIRED)
    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_library(pd_platform_psvita STATIC src/platform/VitaInput.cpp)
    target_include_directories(pd_platform_psvita PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(pd_platform_psvita PUBLIC SceCtrl_stub SceTouch_stub)
    add_executable(PixelDungeonConsoleEditionVita src/main.cpp)
    target_link_libraries(PixelDungeonConsoleEditionVita PRIVATE pd_platform_psvita pd_engine -Wl,--allow-multiple-definition SDL2_mixer modplug xmp xmp-lite opusfile opus ogg vorbisidec PNG::PNG SDL2::SDL2 SDL2_ttf freetype bz2 stdc++ m pthread SceKernelThreadMgr_stub SceProcessmgr_stub SceSysmodule_stub SceNet_stub SceNetCtl_stub SceHttp_stub SceSsl_stub CURL::libcurl OpenSSL::SSL OpenSSL::Crypto z)
    vita_create_self(eboot.bin PixelDungeonConsoleEditionVita)
    set(VITA_DATA_ROOT "${CMAKE_BINARY_DIR}/data/PixelDungeonVita")
    add_custom_target(stage_data ALL COMMAND ${CMAKE_COMMAND} -E make_directory "${VITA_DATA_ROOT}" COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${VITA_DATA_ROOT}/assets")
    # VitaShell rejects LiveArea PNGs unless they are 8-bit palette images.
    # Common encoders silently save low-colour art as 4-bit indexed PNGs,
    # producing installation error 0x8010113D on real hardware.
    function(pd_validate_livearea_png path)
        file(READ "${path}" png_header HEX LIMIT 26)
        string(SUBSTRING "${png_header}" 48 2 png_bit_depth)
        string(SUBSTRING "${png_header}" 50 2 png_color_type)
        if (NOT png_bit_depth STREQUAL "08" OR NOT png_color_type STREQUAL "03")
            message(FATAL_ERROR "LiveArea PNG must be 8-bit indexed: ${path}")
        endif()
    endfunction()
    foreach(livearea_image
        sce_sys/vita/icon0.png
        sce_sys/vita/pic0.png
        sce_sys/vita/livearea/contents/bg0.png
        sce_sys/vita/livearea/contents/startup.png)
        pd_validate_livearea_png("${CMAKE_SOURCE_DIR}/${livearea_image}")
    endforeach()
    # Packaging keeps the historical VPK metadata; runtime assets remain external ux0:data.
    vita_create_vpk(PixelDungeonConsoleEdition.vpk PXLV00001 eboot.bin VERSION 01.70 NAME "PIXEL DUNGEON CONSOLE EDITION"
        FILE assets/font1x.png font1x.png
        FILE assets/Fonts/JeuDePixel-Regular.otf JeuDePixel-Regular.otf
        FILE sce_sys/vita/icon0.png sce_sys/icon0.png
        FILE sce_sys/vita/pic0.png sce_sys/pic0.png
        FILE sce_sys/vita/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
        FILE sce_sys/vita/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
        FILE sce_sys/vita/livearea/contents/startup.png sce_sys/livearea/contents/startup.png)
elseif(PD_PLATFORM STREQUAL "psp")
    if(NOT DEFINED ENV{PSPDEV})
        message(FATAL_ERROR "PSPDEV is not defined. Use cmake/toolchains/psp.cmake in a PSPSDK environment.")
    endif()
    file(TO_CMAKE_PATH "$ENV{PSPDEV}" PSPDEV)
    set(PSPSDK "${PSPDEV}/psp/sdk")
    set(PSP_PORTLIBS "${PSPDEV}/psp")
    add_library(pd_platform_psp STATIC src/platform/psp/PSPInput.cpp)
    target_include_directories(pd_platform_psp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" "${PSPSDK}/include")
    add_executable(PixelDungeonConsoleEditionPSP src/platform/psp/main.cpp)
    target_include_directories(PixelDungeonConsoleEditionPSP PRIVATE "${PSPSDK}/include" "${PSP_PORTLIBS}/include" "${PSP_PORTLIBS}/include/SDL2" "${PSP_PORTLIBS}/include/freetype2")
    target_link_directories(PixelDungeonConsoleEditionPSP PRIVATE "${PSPSDK}/lib" "${PSP_PORTLIBS}/lib")
    target_link_libraries(PixelDungeonConsoleEditionPSP PRIVATE pd_platform_psp pd_engine pd_core
        SDL2_ttf harfbuzz freetype bz2 SDL2_mixer SDL2main SDL2 GL pspvram pspaudio pspvfpu
        pspdisplay pspgu pspge psphprm pspctrl psppower pspdebug pspnet pspnet_apctl
        vorbisfile vorbis ogg modplug png16 z pspuser m "${PSPSDK}/lib/prxexports.o")
    # Keep relocation records and module metadata so psp-prxgen can produce a
    # real PRX. The MG PBP packager encrypts this PRX for retail PSP firmware.
    target_link_options(PixelDungeonConsoleEditionPSP PRIVATE
        "-specs=${PSPSDK}/lib/prxspecs"
        "-Wl,-q,-T${PSPSDK}/lib/linkfile.prx")
    set_target_properties(PixelDungeonConsoleEditionPSP PROPERTIES OUTPUT_NAME "PixelDungeonConsoleEditionPSP.elf")
    add_custom_target(stage_psp_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/userdata")
    add_dependencies(PixelDungeonConsoleEditionPSP stage_psp_assets)
    add_custom_command(TARGET PixelDungeonConsoleEditionPSP POST_BUILD
        COMMAND "${PSPDEV}/bin/psp-fixup-imports" "$<TARGET_FILE:PixelDungeonConsoleEditionPSP>"
        COMMAND "${PSPDEV}/bin/psp-prxgen" "$<TARGET_FILE:PixelDungeonConsoleEditionPSP>" "${CMAKE_BINARY_DIR}/PixelDungeonConsoleEditionPSP.prx"
        COMMAND "${PSPDEV}/bin/mksfoex" -d MEMSIZE=1 "PIXEL DUNGEON CONSOLE EDITION" "${CMAKE_BINARY_DIR}/PARAM.SFO"
        COMMAND "${PSPDEV}/bin/psp-strip" "$<TARGET_FILE:PixelDungeonConsoleEditionPSP>" -o "${CMAKE_BINARY_DIR}/PixelDungeonConsoleEditionPSP_strip.elf"
        COMMAND "${PSPDEV}/bin/pack-pbp" "${CMAKE_BINARY_DIR}/EBOOT.PBP" "${CMAKE_BINARY_DIR}/PARAM.SFO"
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/psp/ICON0.PNG" NULL NULL
            "${CMAKE_CURRENT_SOURCE_DIR}/assets/psp/PIC1.PNG" "${CMAKE_CURRENT_SOURCE_DIR}/assets/psp/SND0.AT3"
            "${CMAKE_BINARY_DIR}/PixelDungeonConsoleEditionPSP_strip.elf" NULL)
elseif(PD_PLATFORM STREQUAL "ps3")
    if(NOT DEFINED ENV{PS3DEV})
        message(FATAL_ERROR "PS3DEV is not defined. Use cmake/toolchains/ps3.cmake through build_ps3.bat.")
    endif()
    set(PS3_PORTLIBS "$ENV{PS3DEV}/portlibs/ppu")
    foreach(ps3_library png mad)
        if(NOT EXISTS "${PS3_PORTLIBS}/lib/lib${ps3_library}.a")
            message(FATAL_ERROR "Missing PS3 portlib: ${PS3_PORTLIBS}/lib/lib${ps3_library}.a")
        endif()
    endforeach()
    target_include_directories(pd_engine PUBLIC "${PS3_PORTLIBS}/include")
    add_library(pd_platform_ps3 STATIC src/platform/ps3/PS3Input.cpp)
    target_include_directories(pd_platform_ps3 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" "${PS3DEV}/ppu/include" "${PS3_PORTLIBS}/include")
    add_executable(PixelDungeonConsoleEditionPS3 src/platform/ps3/main.cpp)
    target_include_directories(pd_engine PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    target_include_directories(PixelDungeonConsoleEditionPS3 PRIVATE "${PS3DEV}/ppu/include" "${PS3_PORTLIBS}/include")
    # PS3DK's maintained rsx-cg-compiler produces CgBinaryProgram blobs for
    # the native cellGcm renderer.  These shaders are not used by SDL.
    include(ps3-self)
    ps3_add_cg_shader_rsxcgc(pd_engine src/platform/ps3/shaders/Renderer2D.vcg)
    ps3_add_cg_shader_rsxcgc(pd_engine src/platform/ps3/shaders/Renderer2D.fcg)
    target_link_libraries(PixelDungeonConsoleEditionPS3 PRIVATE pd_platform_ps3 pd_engine
        "${PS3_PORTLIBS}/lib/libpng.a"
        "${PS3_PORTLIBS}/lib/libmad.a"
        "${PS3_PORTLIBS}/lib/libz.a"
        gcm_cmd gcm_sys rsx sysutil io audio rt lv2 pthread m)
    set_target_properties(PixelDungeonConsoleEditionPS3 PROPERTIES OUTPUT_NAME "PixelDungeonConsoleEditionPS3.elf")
    add_custom_target(stage_ps3_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/userdata")
    add_dependencies(PixelDungeonConsoleEditionPS3 stage_ps3_assets)
elseif(PD_PLATFORM STREQUAL "ps2")
    if(NOT DEFINED ENV{PS2DEV} OR NOT DEFINED ENV{PS2SDK} OR NOT DEFINED ENV{GSKIT})
        message(FATAL_ERROR "PS2DEV, PS2SDK and GSKIT must be defined for the PS2 target.")
    endif()
    set(PS2SDK_PORTS "$ENV{PS2SDK}/ports")
    foreach(ps2_library SDL2 SDL2_ttf png z)
        if(NOT EXISTS "${PS2SDK_PORTS}/lib/lib${ps2_library}.a")
            message(FATAL_ERROR "Missing PS2 port library: ${PS2SDK_PORTS}/lib/lib${ps2_library}.a")
        endif()
    endforeach()
    add_library(pd_platform_ps2 STATIC src/platform/ps2/PS2Input.cpp)
    target_include_directories(pd_platform_ps2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src" "$ENV{PS2SDK}/ee/include")
    add_executable(PixelDungeonConsoleEditionPS2 src/platform/ps2/main.cpp)
    target_include_directories(PixelDungeonConsoleEditionPS2 PRIVATE "$ENV{PS2SDK}/ee/include" "${PS2SDK_PORTS}/include")
    # A local SDL2 build carries the PS2 alpha-test fix. Fall back to the
    # toolchain copy when the local library has not been prepared yet.
    set(PD_PS2_SDL_LIBRARY "${CMAKE_BINARY_DIR}/../sdl-build/libSDL2.a")
    if(NOT EXISTS "${PD_PS2_SDL_LIBRARY}")
        set(PD_PS2_SDL_LIBRARY SDL2)
    endif()
    target_link_libraries(PixelDungeonConsoleEditionPS2 PRIVATE pd_platform_ps2 pd_engine
        # SDL2's PS2 backend requires these static libraries after SDL2.
        ${PD_PS2_SDL_LIBRARY} m patches gskit dmakit ps2_drivers
        SDL2_ttf png z freetype pad -ldebug c)
    set_target_properties(PixelDungeonConsoleEditionPS2 PROPERTIES OUTPUT_NAME "PixelDungeonConsoleEditionPS2.elf")
    add_custom_target(stage_ps2_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/userdata")
    add_dependencies(PixelDungeonConsoleEditionPS2 stage_ps2_assets)
else()
    message(FATAL_ERROR "Platform '${PD_PLATFORM}' is metadata-only or unsupported. Active targets: windows, psvita, ps3.")
endif()
endif()

include(CTest)
if(BUILD_TESTING AND PD_PLATFORM STREQUAL "windows")
    file(GLOB PD_TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*_tests.cpp")
    foreach(test_source IN LISTS PD_TEST_SOURCES)
        get_filename_component(test_name "${test_source}" NAME_WE)
        add_executable(${test_name} "${test_source}")
        target_link_libraries(${test_name} PRIVATE pd_core)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()
