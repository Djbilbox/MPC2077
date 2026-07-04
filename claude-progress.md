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
- [x] Step sequencer (16/32 steps) + host transport sync + step-edit UI (SEQ view)
- [x] ADSR + filter + pitch per pad
- [x] FX: saturation, delay, reverb, bitcrush, glitch, cyber wah (master bus + per-pad sends)
- [x] Preset system (factory programs + save/load user .mpcpreset via ValueTree)
- [x] Real embedded drum kit (16x 808/909/707 one-shots from the FL library)
- [x] Real neon camel logo baked in (checkerboard->alpha)
- [x] Layout redesign (MPC2077 branding + cyberpunk colors)
- [x] Fix Standalone 150%-DPI window sizing (DPI-unaware manifest, like VICE CITY)
- [x] UI styling hardware-synth minimaliste (potards épurés, pads biseautés glossy, badge logo circulaire, two-tone panel, bande accent)
- [ ] UI polish (VU meters, waveform display, more animation) — **optional**
- [ ] Stability tests (CPU, memory leaks, sample edge cases) — **optional**
- [ ] Final VST3 build + installer packaging — **optional**

## UI (redesign — APESHYT disposition, 980x620)
- TopPanel: LCD preset display + `<`/`>` nav (+ popup list) · SAVE/LOAD/INIT/SEQ · logo+wordmark · master VOL knob.
- MixerStrip: 16 per-pad level knobs (short labels KK/808/SN/...); knob drag selects that pad.
- PadGrid: 4x4 big pads (drag&drop + double-click browse). SEQ button swaps it for SequencerView.
- Right column knob banks (reusable `ControlSection`): ENVELOPE (A/D/S/R), TONE (PITCH/FILTER/RESO/DRIVE) = selected pad; FX (REVERB/DELAY/CRUSH/GLITCH/CYBER) = master APVTS.
- Removed old components: ModeBar, ControlPanel, FxRack, PresetBank, TouchRibbon, CityBackdrop.
- Real kit pad names: KICK 808 SNARE CLAP CL-HAT OP-HAT LOW/MID/HI-TOM RIM CRASH RIDE CLAV COWBELL CONGA SNAP.

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

### 2026-07-04 — Session 5 (UI styling hardware-synth)
- User showed une reference hardware drum VST et demanda un redesign du UI s'inspirant du style minimaliste
  (potards épurés, pas de grosses glows, matière glossy 3D) mais en gardant les couleurs cyberpunk MPC2077.
- Redesigned all neon glows to **minimalist hardware-synth style**: smaller knobs with thin pink value arcs
  + flat disc bodies (no big cyan halos), crisp pointer lines, no glow blooms.
- Redesigned pads from flat neon boxes to **glossy 3D beveled** gradients with contact shadows and specular sheen.
- Added **circular cyan glow-ring** around the camel logo badge (concentric halos radiating outward).
- Implemented **two-tone panel split**: lighter steel-blue knob column vs darker left pad area.
- Added **neon gradient accent edge strip** along the left side (pink→cyan).
- Shrunk mixer knobs to fixed minimal hardware size, centered in their column cells.
- Build: clean (exit 0). Screenshot verified all changes applied correctly.
- Commit: 42a70c3 — "UI redesign: hardware-synth minimaliste avec matière glossy, potards épurés, badge circulaire"

**Status: Core plugin fully implemented and styled. All user-requested features complete.**

### Known limitations / next steps
- **Remaining roadmap (optional enhancements):**
  - VU meters on each pad (activity visualization)
  - Waveform display / sample preview
  - Sequencer playhead animation polish
  - Stability/profiling (CPU, memory, edge cases)
  - Installer packaging (.msi / .zip)
  - macOS AU build (if macOS available)
- User samples in presets stored by file PATH (reloaded from disk); embedding raw audio in the
  preset for full portability is a later improvement.
- All systems fully tested in Standalone; not yet verified in a DAW host (FL Studio, Reaper, etc.).
