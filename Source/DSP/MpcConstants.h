#pragma once

namespace mpc
{
    static constexpr int   kNumPads   = 16;   // 4x4 MPC grid
    static constexpr int   kMaxSteps  = 32;   // sequencer resolution ceiling
    static constexpr int   kNumVoices = 24;   // polyphony (pads can overlap)
    static constexpr int   kBaseMidiNote = 36; // pad 0 = C1 (GM kick), pad i = 36+i
    static constexpr int   FactoryPresetCount = 50;

    // Parameter IDs (global / automatable — APVTS)
    namespace pid
    {
        static constexpr const char* master   = "master";
        static constexpr const char* mode      = "mode";
        static constexpr const char* swing     = "swing";
        static constexpr const char* humanize  = "humanize";
        static constexpr const char* pitchWheel = "pitchwheel";
        static constexpr const char* modWheel   = "modwheel";
        // master FX (right rack)
        static constexpr const char* fxReverb  = "fx_reverb";
        static constexpr const char* fxGlitch  = "fx_glitch";
        static constexpr const char* fxCrush   = "fx_crush";
        static constexpr const char* fxDelay   = "fx_delay";
        static constexpr const char* fxCyber   = "fx_cyber";
    }
}
