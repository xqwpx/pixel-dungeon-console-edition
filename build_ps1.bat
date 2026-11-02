@echo off
setlocal EnableExtensions
title PIXEL DUNGEON CONSOLE EDITION - PS1 Build

set "PROJECT_ROOT=%~dp0"
set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"
if "%PSN00BSDK%"=="" set "PSN00BSDK=E:\Toolchains\PSn00bSDK\PSn00bSDK-0.24-win32"
if "%PSN00BSDK_TC%"=="" set "PSN00BSDK_TC=%PSN00BSDK%"
if not exist "%PSN00BSDK%\lib\libpsn00b\cmake\sdk.cmake" (echo ERROR: PSn00bSDK not found at "%PSN00BSDK%" & exit /b 1)
if not exist "%PSN00BSDK_TC%\bin\mipsel-none-elf-g++.exe" (echo ERROR: PS1 GCC toolchain not found at "%PSN00BSDK_TC%" & exit /b 1)
set "PSN00BSDK_LIBS=%PSN00BSDK%\lib\libpsn00b"
set "PATH=%PSN00BSDK%\bin;%PATH%"

pushd "%PROJECT_ROOT%" >nul || exit /b 1
cmake -S . -B build\ps1 -G Ninja -DCMAKE_TOOLCHAIN_FILE="%PSN00BSDK_LIBS%\cmake\sdk.cmake" -DPD_PLATFORM=ps1 -DCMAKE_BUILD_TYPE=Release -DPSN00BSDK_TC="%PSN00BSDK_TC%" -DPSN00BSDK_TARGET=mipsel-none-elf
if errorlevel 1 goto :failed
cmake --build build\ps1 --target PixelDungeonConsoleEditionPS1Iso
if errorlevel 1 goto :failed
if not exist dist\ps1 mkdir dist\ps1
copy /Y build\ps1\PixelDungeonConsoleEditionPS1.exe dist\ps1\PixelDungeonConsoleEditionPS1.exe >nul
if errorlevel 1 goto :failed
copy /Y build\ps1\PixelDungeonConsoleEditionPS1.bin dist\ps1\PixelDungeonConsoleEditionPS1.bin >nul
if errorlevel 1 goto :failed
copy /Y build\ps1\PixelDungeonConsoleEditionPS1.cue dist\ps1\PixelDungeonConsoleEditionPS1.cue >nul
if errorlevel 1 goto :failed
echo BUILD SUCCESS
echo EXE: dist\ps1\PixelDungeonConsoleEditionPS1.exe
echo CD:  dist\ps1\PixelDungeonConsoleEditionPS1.bin
popd >nul
exit /b 0

:failed
echo BUILD FAILED
popd >nul
exit /b 1
