@echo off
echo ============================================================
echo  MPC2077 -- Build Release
echo ============================================================

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

set "CMAKE=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

cd /d "C:\MPC2077"

"%CMAKE%" --build build --config Release -j4

if %errorlevel%==0 (
    echo.
    echo ============================================================
    echo  BUILD SUCCESS!
    echo ============================================================
    echo.

    set "SRC=build\MPC2077_artefacts\Release\VST3\MPC2077.vst3"
    set "DST=C:\Program Files\Common Files\VST3\MPC2077.vst3"

    if exist "%SRC%" (
        xcopy /E /I /Y "%SRC%" "%DST%" >nul 2>&1
        if %errorlevel%==0 (
            echo  VST3 installed to: %DST%
        ) else (
            set "USR=%LOCALAPPDATA%\Programs\Common\VST3\MPC2077.vst3"
            xcopy /E /I /Y "%SRC%" "%USR%" >nul 2>&1
            echo  VST3 installed to: %USR%
        )
    )

    echo.
    echo  Standalone: build\MPC2077_artefacts\Release\Standalone\MPC2077.exe
    echo  VST3:       build\MPC2077_artefacts\Release\VST3\MPC2077.vst3
) else (
    echo.
    echo  BUILD FAILED -- see output above
)

echo BUILD_EXIT=%errorlevel%
