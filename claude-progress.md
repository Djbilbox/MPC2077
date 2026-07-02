# MPC2077 — Progress Journal

> Futuristic drum machine — VST3/AU plugin (JUCE / C++17). Cyberpunk neon aesthetic.
> This file is the project's source of truth. Each session: read → pick the next
> unchecked task → implement → test the build → update this file → commit.

## Toolchain (verified 2026-07-02)
- JUCE: `C:/JUCE` (8.0.x, complete — the only valid JUCE on this machine)
- Compiler: MSVC 19.44 via **VS Build Tools 2022** (`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`)
- CMake + Ninja: bundled inside BuildTools (see `mpc_configure.bat`)
- Configure: `mpc_configure.bat`  → generates `build/`
- Build: `mpc_build.bat` → `build/MPC2077_artefacts/Release/{VST3,Standalone}/`
- Install for FL (admin): `install_vst3_admin.bat` → `C:\Program Files\Common Files\VST3\`
- Project location: **`C:\MPC2077\`** (dedicated, space-free folder — avoids Ninja issues)
- Design reference assets: `C:\Users\djbil\Desktop\Djbilbox BEATS\PLUGIN VST by Djbilbox BEATS\5. MPC2077\`

## Architecture
- DSP strictly separated from UI (DSP includes no UI headers).
- `Source/DSP/`: DrumEngine (16 voices), PadVoice, StepSequencer, FxChain, DrumSynth (procedural default sounds).
- `Source/UI/`: MPC2077LookAndFeel (custom neon), Theme (colors), Components/ (PadGrid, FxRack, PresetBank, ModeBar, TouchRibbon, CityBackdrop, ControlPanel).
- One PluginProcessor / PluginEditor.
- State saved via ValueTree (global APVTS params + per-pad state + sequencer pattern).

## Palette (design brief)
- Background: `#05000F` (deep cyberpunk black)
- Electric cyan: `#00F5FF`  — Neon pink: `#FF2D78`  — Chrome yellow: `#FFE600`

## ROADMAP
- [x] Project structure CMake/JUCE + init.sh
- [x] Base PluginProcessor (stereo, sample-rate agnostic)
- [x] Sample playback engine (16 polyphonic voices)
- [x] 16 UI pads with custom neon LookAndFeel
- [x] Drag & drop sample onto pad + file browser (double-click)
- [x] Step sequencer (16/32 steps) + host transport sync  (engine done; step-EDIT UI still TODO)
- [x] ADSR + filter + pitch per pad
- [x] FX: saturation, delay, reverb, bitcrush, glitch, cyber wah (master bus + per-pad sends)
- [x] Preset system (factory programs + save/load user .mpcpreset via ValueTree)
- [ ] UI polish (step-sequencer editor view, VU meters, waveform display, animations)
- [ ] Stability tests (CPU, memory leaks, sample edge cases)
- [ ] Final VST3 build (+ AU if macOS available) + installer packaging

## Journal
### 2026-07-02 — Session 1 (Init + core)
- Initial state: no code, only design assets (HTML mockup + 2 captures + brief + logo JPEG) in the
  Desktop folder. JUCE/cmake/ninja/git present and verified.
- Decision: project in `C:\MPC2077\` (like BIGBASS) — space-free paths for Ninja.
- Decision: **procedural** embedded drum sounds (kick/snare/hats/clap/toms/perc…) so the plugin is
  audible out of the box with NO external WAV dependency; user sample loading (drag&drop + browser)
  works on top. (Same fallback strategy as BIGBASS.)
- Decision on params: APVTS for automatable global params (master, 5 FX, swing/humanize, pitch, mod,
  mode). Per-pad params (sample, gain, pan, pitch, filter, ADSR) live in engine state edited by the
  UI when a pad is selected (standard for a pad sampler); sequencer pattern stored separately.
- Implemented the full DSP layer and full neon UI in one pass.
- Build: first attempt failed on a single `const` error (`captureState()` calling non-const
  `apvts.copyState()`) → made it non-const. Rebuild in progress.
- LANGUAGE: user asked that everything be produced in English. Converted this journal to English;
  code comments were already English. TODO: translate the French strings in `install_vst3_admin.bat`.

### Known limitations / next steps
- Step-sequencer EDIT UI not built yet (patterns play from factory presets; engine + sync are done).
- User samples in presets are stored by file PATH (reloaded from disk); embedding raw audio in the
  preset for full portability is a later improvement.
- Not yet visually verified in a host — screenshot the Standalone once it builds.
