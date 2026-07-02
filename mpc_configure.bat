@echo off
echo ============================================================
echo  MPC2077 -- CMake Configure
echo ============================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

set "CMAKE=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

cd /d "C:\MPC2077"

"%CMAKE%" -G Ninja -S . -B build -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR="C:/JUCE"

echo.
echo CONFIGURE_EXIT=%errorlevel%
