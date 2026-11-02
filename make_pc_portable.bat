@echo off
setlocal
cd /d "%~dp0"

echo ========================================
echo  PIXEL DUNGEON CONSOLE EDITION - Portable Windows Build
echo ========================================
echo.

where python >nul 2>nul || (echo ERROR: Python was not found in PATH.& goto failed)
if not exist "%ProgramFiles%\CMake\bin\cmake.exe" goto check_cmake
set "PATH=%ProgramFiles%\CMake\bin;%PATH%"
:check_cmake
where cmake >nul 2>nul || (echo ERROR: CMake was not found in PATH.& goto failed)

echo Building Windows executable and staging runtime files...
python tools\pd.py package --platform windows --config debug
if errorlevel 1 goto failed

echo.
echo SUCCESS
echo Portable folder:
echo   %CD%\dist\windows
echo Launch with:
echo   dist\windows\run_pixel_dungeon.bat
goto end

:failed
echo.
echo BUILD FAILED
echo Install a native Windows CMake toolchain and SDL2, SDL2_ttf,
echo SDL2_mixer and libpng development packages, then run this file again.

:end
pause
