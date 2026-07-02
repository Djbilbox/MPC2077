#pragma once

#include <JuceHeader.h>
#include "MpcConstants.h"

namespace mpc
{
/**
    Procedural cyberpunk drum kit.

    Renders 16 default one-shot drum sounds at the host sample rate so the plugin
    is immediately playable with no external WAV dependency (users can still drop
    their own samples on top). Pure DSP — no UI knowledge.
*/
struct DrumSynth
{
    /** Render the full default kit (mono buffers) at the given sample rate. */
    static void renderKit (std::array<juce::AudioBuffer<float>, kNumPads>& out, double sampleRate);

    /** Render a single pad type (0..15). Kept public for "reset pad to default". */
    static void renderPad (int pad, juce::AudioBuffer<float>& out, double sampleRate);

    static juce::String defaultName  (int pad);
    static int          defaultChoke (int pad); // 0 = none; hats share group 1
};
}
