#!/usr/bin/env bash
# ============================================================
#  MPC2077 - init / build / smoke-test
#  Usage:  bash init.sh          (configure + build)
#          bash init.sh clean    (wipe build/ first)
# ============================================================
set -e

ROOT="C:/MPC2077"
VCVARS="C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/VC/Auxiliary/Build/vcvars64.bat"
CMAKE="C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"

cd "$ROOT"

if [ "$1" == "clean" ]; then
    echo "[init] cleaning build/"
    rm -rf build
fi

echo "[init] configuring (Ninja / Release)"
cmd.exe /c "mpc_configure.bat"

echo "[init] building"
cmd.exe /c "mpc_build.bat"

echo "[init] artefacts:"
ls -R build/MPC2077_artefacts/Release 2>/dev/null || echo "  (build failed - see log above)"

echo "[init] done."
