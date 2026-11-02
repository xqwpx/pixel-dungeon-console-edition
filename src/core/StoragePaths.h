#pragma once
#include <string>

#ifndef PD_USERDATA_ROOT
#if defined(PD_PLATFORM_PSP)
#define PD_USERDATA_ROOT "ms0:/PSP/GAME/PixelDungeon/userdata/"
#else
#define PD_USERDATA_ROOT ""
#endif
#endif

namespace pdv { namespace StoragePaths {
// CMake assigns one root per target platform:
// Windows: userdata/ next to the portable executable
// PS Vita: ux0:data/PixelDungeonVita/ (unchanged)
// PS3: /dev_hdd0/game/PDXCE0001/USRDIR/userdata/
// PS2: host:userdata/ next to the staged ELF on the host filesystem
inline const std::string& root() { static const std::string value = PD_USERDATA_ROOT; return value; }
inline const std::string& saveGame() { static const std::string value = std::string(PD_USERDATA_ROOT) + "save.dat"; return value; }
inline const std::string& bones() { static const std::string value = std::string(PD_USERDATA_ROOT) + "bones.dat"; return value; }
inline const std::string& profile() { static const std::string value = std::string(PD_USERDATA_ROOT) + "profile.dat"; return value; }
} }
