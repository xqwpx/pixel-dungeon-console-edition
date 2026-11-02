#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#if defined(PD_PLATFORM_PSP)
#include <pspiofilemgr.h>
#endif

namespace pdv {
#if defined(PD_PLATFORM_PSP)
inline bool pspLaunchedFromDisc() {
    // The same executable is used both as a memory-stick homebrew PBP and
    // inside the optional UMD ISO layout.
    static const bool launchedFromDisc = [] {
        SceIoStat stat{};
        return sceIoGetstat("disc0:/PSP_GAME/PARAM.SFO", &stat) >= 0;
    }();
    return launchedFromDisc;
}

inline const char* pspAssetRoot() {
    return pspLaunchedFromDisc()
        ? "disc0:/PSP_GAME/USRDIR/assets/"
        : "ms0:/PSP/GAME/PixelDungeon/assets/";
}

inline std::string pspAssetPath(const std::string& relativePath) {
    std::string path = std::string(pspAssetRoot()) + relativePath;
    if (pspLaunchedFromDisc()) {
        // PSP's UMD ISO9660 driver exposes primary volume names in uppercase.
        // PPSSPP follows this behavior for sceIoOpen(), unlike Windows paths.
        const std::size_t pathStart = path.find(":/") + 2;
        std::transform(path.begin() + static_cast<std::ptrdiff_t>(pathStart), path.end(),
                       path.begin() + static_cast<std::ptrdiff_t>(pathStart),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    }
    return path;
}
#endif

inline std::string resolveAssetPath(const std::string& path) {
#if defined(PD_PLATFORM_WINDOWS)
    const std::string dataPrefix = "ux0:data/PixelDungeonVita/assets/";
    if (path.compare(0, dataPrefix.size(), dataPrefix) == 0)
        return std::string("assets/") + path.substr(dataPrefix.size());
    if (path == "app0:JeuDePixel-Regular.otf")
        return "assets/Fonts/JeuDePixel-Regular.otf";
    if (path == "app0:font1x.png")
        return "assets/font1x.png";
#elif defined(PD_PLATFORM_PSP)
    const std::string dataPrefix = "ux0:data/PixelDungeonVita/assets/";
    if (path.compare(0, dataPrefix.size(), dataPrefix) == 0)
        return pspAssetPath(path.substr(dataPrefix.size()));
    if (path == "app0:JeuDePixel-Regular.otf")
        return pspAssetPath("Fonts/JeuDePixel-Regular.otf");
    if (path == "app0:font1x.png")
        return pspAssetPath("font1x.png");
    if (path.compare(0, 7, "assets/") == 0)
        return pspAssetPath(path.substr(7));
#elif defined(PD_PLATFORM_PS3) || defined(PD_PLATFORM_PS2)
    const std::string dataPrefix = "ux0:data/PixelDungeonVita/assets/";
    if (path.compare(0, dataPrefix.size(), dataPrefix) == 0)
        return std::string(PD_ASSET_ROOT) + path.substr(dataPrefix.size());
    if (path == "app0:JeuDePixel-Regular.otf")
        return std::string(PD_ASSET_ROOT) + "Fonts/JeuDePixel-Regular.otf";
    if (path == "app0:font1x.png")
        return std::string(PD_ASSET_ROOT) + "font1x.png";
#endif
    return path;
}
} // namespace pdv
