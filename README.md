# MPC2077 — Cyberpunk Drum Machine VST3/AU

**MPC2077 by DJBILBOX BEATS** — A 16-pad polyphonic drum machine plugin for Windows (VST3) and macOS (AU).

---

## Features

✨ **16 MPC-style pads** with real 808/909/707 drum samples  
✨ **24-voice polyphony** with per-pad ADSR envelopes + Zavalishin SVF filters  
✨ **Step sequencer** (16/32 steps) with host sync + swing/humanize  
✨ **Master FX chain**: reverb, delay, bitcrush, glitch, cyber wah  
✨ **50 factory presets** with unique groove patterns  
✨ **Save/load user presets** (.mpcpreset format)  
✨ **Hardware-synth minimaliste UI** — glossy 3D pads, minimalist knobs, neon badge logo  
✨ **Cyberpunk palette**: cyan #00F5FF, pink #FF2D78, dark #05000F  

---

## Downloads

👉 **[MPC2077 Releases](https://github.com/djbilbox/MPC2077/releases)**

- **Windows**: `MPC2077-Windows-v*.zip` → VST3 + Standalone .exe
- **macOS**: `MPC2077-macOS-v*.dmg` → AU plugin + Standalone .app

---

## Installation

### Windows (VST3 + Standalone)
1. Download `MPC2077-Windows-v*.zip`
2. Extract the archive
3. **Standalone**: Run `MPC2077.exe` directly
4. **VST3 Plugin**: Copy `MPC2077.vst3` to `C:\Program Files\Common Files\VST3\`

### macOS (AU + Standalone)
1. Download `MPC2077-macOS-v*.dmg`
2. Open the DMG and drag files to their destinations:
   - **AU Plugin**: → `/Library/Audio/Plug-Ins/Components/MPC2077.component`
   - **Standalone**: → `/Applications/MPC2077.app`
3. Restart your DAW

---

## User Guide

📖 **See [MANUAL.md](MANUAL.md)** for complete documentation:
- Interface walkthrough
- 16 pads + mixer strip + knob sections
- Step sequencer editor
- 50 factory presets
- Save/load custom presets
- Tips & tricks

---

## Technical Specs

| Spec | Value |
|------|-------|
| **Pads** | 16 (4×4 MPC grid) |
| **Polyphony** | 24 voices |
| **Built-in Samples** | 16 × 808/909/707 one-shots |
| **Factory Presets** | 50 |
| **Sequencer** | 16/32 steps, host-synced + internal clock |
| **Filters** | SVF (LP/HP/BP) per pad |
| **Master FX** | Reverb, Delay, Bitcrush, Glitch, Cyber Wah |
| **Framework** | JUCE 8.0 |
| **C++ Standard** | C++17 |
| **Format (Windows)** | VST3, Standalone |
| **Format (macOS)** | AU (Audio Unit), Standalone |

---

## Build from Source

### Windows
**Requirements**: JUCE 8 at `C:/JUCE`, VS Build Tools 2022, CMake 3.22+, Ninja

```bash
mpc_configure.bat   # CMake configure (Ninja, Release)
mpc_build.bat       # build VST3 + Standalone
```

Output: `build/MPC2077_artefacts/Release/{VST3,Standalone}/`

To install VST3 to your system:
```bash
install_vst3_admin.bat   # requires admin UAC prompt
```

### macOS
**Requirements**: JUCE 8 at `~/JUCE`, Xcode, CMake, Ninja

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR=~/JUCE
cd build
ninja
```

Output: `build/MPC2077_artefacts/Release/{AU,Standalone}/`

---

## Releases via GitHub Actions

This repo has **automated CI/CD** for building releases:

- **Windows build**: Triggered on `git push` with a `v*` tag (e.g., `v1.0.0`)
  - Runs on GitHub's Windows VM
  - Builds VST3 + Standalone
  - Creates ZIP artifact

- **macOS build**: Triggered on `v*` tag
  - Runs on GitHub's macOS VM
  - Builds AU + Standalone
  - Creates DMG artifact

**To release:**
```bash
git tag v1.0.0
git push origin v1.0.0
```
Then check the [Releases](https://github.com/djbilbox/MPC2077/releases) page for the Windows ZIP and macOS DMG.

---

## Project Structure

```
C:\MPC2077\
├── Source/
│   ├── DSP/              # engine, voices, sequencer, FX (no UI deps)
│   ├── UI/               # custom neon LookAndFeel + components
│   ├── PluginProcessor.cpp/h
│   └── PluginEditor.cpp/h
├── assets/
│   ├── samples/          # 16 × 808/909/707 one-shot WAVs
│   ├── logo.png          # neon camel logo
│   └── Standalone.manifest   # DPI-unaware (Windows HiDPI fix)
├── CMakeLists.txt
├── mpc_configure.bat     # Windows configure script
├── mpc_build.bat         # Windows build script
├── install_vst3_admin.bat
├── .github/workflows/    # GitHub Actions CI/CD
├── MANUAL.md             # user guide (English)
├── README.md             # this file
└── claude-progress.md    # development journal
```

---

## Contact & Support

**Email**: djbilbox@gmail.com  
**Website**: [djbilbox.com](https://djbilbox.com)  
**Issues**: [GitHub Issues](https://github.com/djbilbox/MPC2077/issues)

---

## License

© 2026 DJBILBOX BEATS. All rights reserved.
