@echo off
REM ============================================================
REM  MPC2077 - Install the VST3 into the folder FL Studio scans
REM  Double-click this file and accept the Windows prompt (UAC).
REM ============================================================

net session >nul 2>&1
if %errorlevel% NEQ 0 (
    echo Requesting administrator rights...
    powershell -Command "Start-Process -FilePath '%~f0' -Verb RunAs"
    exit /b
)

set "SRC=%~dp0build\MPC2077_artefacts\Release\VST3\MPC2077.vst3"
set "DST=C:\Program Files\Common Files\VST3\MPC2077.vst3"

echo.
echo  Source : %SRC%
echo  Target : %DST%
echo.

if not exist "%SRC%" (
    echo  ERROR: VST3 not found. Run mpc_build.bat first.
    pause
    exit /b 1
)

if exist "%DST%" rmdir /S /Q "%DST%"

robocopy "%SRC%" "%DST%" /MIR /NJH /NJS /NDL /NFL >nul

if exist "%DST%\Contents\x86_64-win\MPC2077.vst3" (
    echo.
    echo  ============================================
    echo   INSTALL SUCCESSFUL
    echo  ============================================
    echo  In FL Studio: ADD ^> Refresh the plugin list
    echo  (fast scan), then search for MPC2077.
) else (
    echo  COPY FAILED.
)
echo.
pause
