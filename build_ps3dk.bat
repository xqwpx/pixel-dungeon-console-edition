@echo off
setlocal EnableExtensions
title PIXEL DUNGEON CONSOLE EDITION - PS3DK Build

set "PROJECT_ROOT=%~dp0"
set "PROJECT_ROOT=%PROJECT_ROOT:~0,-1%"
pushd "%PROJECT_ROOT%" >nul || (
    echo ERROR: Could not open the project directory.
    goto :failed
)

if "%PS3DK%"=="" set "PS3DK=E:\Toolchains\PS3DK\v0.25.0\ps3-sdk-v0.25.0-windows-x86_64"
if not exist "%PS3DK%\cmake\ps3-ppu-toolchain.cmake" (
    echo ERROR: PS3DK was not found at "%PS3DK%".
    goto :failed
)

for /f "delims=" %%I in ('where cmake 2^>nul') do if not defined HOST_CMAKE set "HOST_CMAKE=%%I"
if not defined HOST_CMAKE (
    echo ERROR: CMake is not available in PATH.
    goto :failed
)

set "PS3DEV=%PS3DK%"
set "PSL1GHT=%PS3DK%"
set "PATH=%PS3DK%\bin;%PS3DK%\ppu\bin;%PS3DK%\make\bin;%PATH%"
set "PROJECT_ROOT_FWD=%PROJECT_ROOT:\=/%"

if not exist "%PS3DK%\make\bin\make.exe" (
    echo ERROR: PS3DK make.exe is missing.
    goto :failed
)

echo.
echo PS3DK: %PS3DK%
echo Configuring...
"%HOST_CMAKE%" -S "%PROJECT_ROOT%" -B "%PROJECT_ROOT%\build\ps3dk\release" -G "Unix Makefiles" "-DCMAKE_MAKE_PROGRAM=%PS3DK%\make\bin\make.exe" "-DCMAKE_TOOLCHAIN_FILE=%PROJECT_ROOT_FWD%/cmake/toolchains/ps3dk.cmake" -DPD_PLATFORM=ps3 -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto :failed
echo Building...
"%HOST_CMAKE%" --build "%PROJECT_ROOT%\build\ps3dk\release" --parallel
if errorlevel 1 goto :failed

if not exist dist\ps3dk mkdir dist\ps3dk
if not exist build\ps3dk\pkg\USRDIR\userdata mkdir build\ps3dk\pkg\USRDIR\userdata
copy /Y build\ps3dk\release\PixelDungeonConsoleEditionPS3.elf dist\ps3dk\PixelDungeonConsoleEditionPS3.elf >nul
"%PS3DK%\ppu\bin\powerpc64-ps3-elf-strip.exe" build\ps3dk\release\PixelDungeonConsoleEditionPS3.elf -o build\ps3dk\PixelDungeonConsoleEditionPS3.stripped.elf
if errorlevel 1 goto :failed
"%PS3DK%\bin\sprxlinker.exe" build\ps3dk\PixelDungeonConsoleEditionPS3.stripped.elf
if errorlevel 1 goto :failed
"%PS3DK%\bin\make_self_npdrm.exe" build\ps3dk\PixelDungeonConsoleEditionPS3.stripped.elf build\ps3dk\pkg\USRDIR\EBOOT.BIN UP0001-PDXCE0001_00-0000000000000000
if errorlevel 1 goto :failed
"%PS3DK%\bin\sfo.exe" --title "PIXEL DUNGEON CONSOLE EDITION" --appid PDXCE0001 -f "%PS3DK%\bin\sfo.xml" build\ps3dk\pkg\PARAM.SFO
if errorlevel 1 goto :failed
copy /Y "%PS3DK%\bin\ICON0.PNG" build\ps3dk\pkg\ICON0.PNG >nul
xcopy /E /I /Y build\ps3dk\release\assets build\ps3dk\pkg\USRDIR\assets >nul
type nul > build\ps3dk\pkg\USRDIR\userdata\.keep
"%PS3DK%\bin\pkg.exe" --contentid UP0001-PDXCE0001_00-0000000000000000 build\ps3dk\pkg build\ps3dk\PixelDungeonConsoleEditionPS3.debug.pkg
if errorlevel 1 goto :failed
copy /Y build\ps3dk\PixelDungeonConsoleEditionPS3.debug.pkg dist\ps3dk\PixelDungeonConsoleEditionPS3.pkg >nul
"%PS3DK%\bin\package_finalize.exe" dist\ps3dk\PixelDungeonConsoleEditionPS3.pkg
if errorlevel 1 goto :failed

echo BUILD SUCCESS
echo ELF: dist\ps3dk\PixelDungeonConsoleEditionPS3.elf
echo HEN PKG: dist\ps3dk\PixelDungeonConsoleEditionPS3.pkg
echo Saves: /dev_hdd0/game/PDXCE0001/USRDIR/userdata/
goto :end

:failed
echo BUILD FAILED

:end
popd >nul
pause
