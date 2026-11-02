@echo off
setlocal EnableExtensions
title PIXEL DUNGEON CONSOLE EDITION - PS Vita VPK Build

echo ========================================
echo  PIXEL DUNGEON CONSOLE EDITION - PS Vita VPK Build
echo ========================================

where wsl >nul 2>nul
if errorlevel 1 (
    echo ERROR: WSL is not available.
    goto :failed
)

if "%VITASDK_WSL%"=="" set "VITASDK_WSL=/usr/local/vitasdk"
pushd "%~dp0"
for /f "delims=" %%I in ('wsl wslpath -a -u "%CD%"') do set "WSL_PROJECT=%%I"
popd

if "%WSL_PROJECT%"=="" (
    echo ERROR: Could not convert the project path for WSL.
    goto :failed
)

echo VitaSDK: %VITASDK_WSL%
echo Building VPK...
wsl env VITASDK=%VITASDK_WSL% bash -lc 'set -e; export PATH="$VITASDK/bin:$PATH"; cd %WSL_PROJECT%; test -f "$VITASDK/share/vita.toolchain.cmake"; python3 tools/check_assets.py; cmake -S . -B build/psvita/vpk-release -DCMAKE_TOOLCHAIN_FILE="$VITASDK/share/vita.toolchain.cmake" -DPD_PLATFORM=psvita -DCMAKE_BUILD_TYPE=Release; cmake --build build/psvita/vpk-release --parallel; mkdir -p dist/psvita; cp build/psvita/vpk-release/PixelDungeonConsoleEdition.vpk dist/psvita/PixelDungeonConsoleEdition.vpk'
if errorlevel 1 goto :failed

echo.
echo BUILD SUCCESS
echo VPK: dist\psvita\PixelDungeonConsoleEdition.vpk
echo Assets: build\psvita\vpk-release\data\PixelDungeonVita
echo Copy PixelDungeonVita from Assets to ux0:data on the Vita.
goto :end

:failed
echo.
echo BUILD FAILED
echo Ensure WSL has VitaSDK at %VITASDK_WSL%.

:end
pause
