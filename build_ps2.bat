@echo off
setlocal EnableExtensions
title PIXEL DUNGEON CONSOLE EDITION - PS2 Build

set "PROJECT_ROOT=%~dp0"
set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"
if "%PS2DEV%"=="" set "PS2DEV=E:\Toolchains\PS2DEV"
if "%PS2SDK%"=="" set "PS2SDK=%PS2DEV%\ps2sdk"
if "%GSKIT%"=="" set "GSKIT=%PS2DEV%\gsKit"
if not exist "%PS2SDK%\ps2dev.cmake" (echo ERROR: PS2SDK not found at "%PS2SDK%" & exit /b 1)
if not exist "%GSKIT%\lib\libgskit.a" (echo ERROR: gsKit not found at "%GSKIT%" & exit /b 1)
set "PATH=%PS2DEV%\ee\bin;%PS2DEV%\bin;%PS2SDK%\bin;%PATH%"
for /f "delims=" %%I in ('where mingw32-make 2^>nul') do if not defined PS2_MAKE set "PS2_MAKE=%%I"
for /f "delims=" %%I in ('where make 2^>nul') do if not defined PS2_MAKE set "PS2_MAKE=%%I"
if not defined PS2_MAKE if exist "E:\Toolchains\PS3DK\v0.25.0\ps3-sdk-v0.25.0-windows-x86_64\make\bin\make.exe" set "PS2_MAKE=E:\Toolchains\PS3DK\v0.25.0\ps3-sdk-v0.25.0-windows-x86_64\make\bin\make.exe"
if not defined PS2_MAKE (echo ERROR: GNU Make was not found. Install mingw32-make or set PS2_MAKE. & exit /b 1)
pushd "%PROJECT_ROOT%" >nul || exit /b 1
if not exist "build\ps2\sdl-build\CMakeCache.txt" (echo ERROR: Local PS2 SDL build is missing. Prepare build\ps2\sdl-build first. & goto :failed)
cmake --build build\ps2\sdl-build --parallel
if errorlevel 1 goto :failed
if not exist "build\ps2\sdl-build\libSDL2.a" (echo ERROR: Local PS2 SDL library was not produced. & goto :failed)
cmake -S . -B build\ps2\release -G "Unix Makefiles" "-DCMAKE_MAKE_PROGRAM=%PS2_MAKE%" -DCMAKE_TOOLCHAIN_FILE="%PS2SDK%\ps2dev.cmake" -DPD_PLATFORM=ps2 -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :failed
cmake --build build\ps2\release --parallel
if errorlevel 1 goto :failed
if not exist dist\ps2 mkdir dist\ps2
copy /Y build\ps2\release\PixelDungeonConsoleEditionPS2.elf dist\ps2\PixelDungeonConsoleEditionPS2.elf >nul
xcopy /E /I /Y build\ps2\release\assets dist\ps2\assets >nul
if not exist dist\ps2\userdata mkdir dist\ps2\userdata
echo BUILD SUCCESS
echo ELF: dist\ps2\PixelDungeonConsoleEditionPS2.elf
echo Assets: dist\ps2\assets
echo Saves: host:userdata/ (the userdata folder beside the staged ELF)
popd >nul
exit /b 0
:failed
echo BUILD FAILED
popd >nul
exit /b 1
