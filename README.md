# MPC2077 — by DJBILBOX BEATS

A futuristic **cyberpunk drum machine** — VST3 / Standalone (JUCE, C++17).

16 MPC-style pads · step sequencer (16/32) · per-pad pitch/filter/ADSR ·
master FX (saturation, delay, reverb, bitcrush, glitch) · neon custom UI.

## Build (Windows)
```
mpc_configure.bat   :: CMake configure (Ninja, Release)
mpc_build.bat       :: build + install VST3
```
Artefacts land in `build/MPC2077_artefacts/Release/{VST3,Standalone}/`.
To make FL Studio see it, double-click `install_vst3_admin.bat` (one UAC prompt),
then in FL: **AJOUT → Rafraîchir la liste des plugins**.

Requirements: JUCE 8 at `C:/JUCE`, VS Build Tools 2022 (MSVC + bundled CMake/Ninja).

## Layout
- `Source/DSP/`  — engine, voices, sequencer, FX, procedural drum synth (no UI deps)
- `Source/UI/`   — custom neon LookAndFeel + modular components
- See `claude-progress.md` for the full roadmap and dev journal.
