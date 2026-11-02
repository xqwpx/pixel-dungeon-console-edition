@echo off
setlocal
echo ========================================
echo  PIXEL DUNGEON CONSOLE EDITION - Windows Build
echo ========================================
echo Checking tools...
where python >nul 2>nul || (echo Python not found & goto failed)
if not exist "%ProgramFiles%\CMake\bin\cmake.exe" goto check_cmake
set "PATH=%ProgramFiles%\CMake\bin;%PATH%"
:check_cmake
where cmake >nul 2>nul || (echo CMake not found & goto failed)
echo OK Python
echo OK CMake
python tools\pd.py build --platform windows --config debug
if errorlevel 1 goto failed
echo BUILD SUCCESS
echo Executable: build\windows\debug\PIXEL DUNGEON CONSOLE EDITION.exe
goto end
:failed
echo BUILD FAILED
echo See errors above.
:end
pause
