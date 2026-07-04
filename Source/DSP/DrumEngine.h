#pragma once

#include <JuceHeader.h>
#include "MpcConstants.h"

namespace mpc
{
//==============================================================================
/** Per-pad sound settings (edited by the UI, saved in presets). Not automatable. */
struct PadSettings
{
    juce::String name        { "PAD" };
    juce::String sampleName;                 // file name if a user sample is loaded
    juce::String filePath;                   // absolute path for preset reload
    juce::AudioBuffer<float> sample;         // mono or stereo source
    double sourceRate = 44100.0;
    bool   isUserSample = false;
    bool   isProcedural = true;   // true = synthesized fallback (rate-dependent)

    // pitch
    float pitchSemis = 0.0f;   // -24..24
    float fineCents  = 0.0f;   // -100..100
    float glide      = 0.0f;   // 0..1  downward pitch-envelope depth (zap/slide)
    float startPos   = 0.0f;   // 0..1  playback start offset
    bool  reverse    = false;

    // amp envelope (seconds; sustain 0..1)
    float attack  = 0.001f;
    float decay   = 1.20f;
    float sustain = 0.0f;
    float release = 0.08f;

    // filter
    int   filterType = 0;      // 0 LP, 1 HP, 2 BP
    float cutoff = 1.0f;       // 0..1 -> 30..18000 Hz (log)
    float reso   = 0.10f;      // 0..1

    // level / routing
    float gain = 0.85f;        // 0..1.5
    float pan  = 0.0f;         // -1..1
    float drive = 0.0f;        // 0..1 per-pad saturation
    float reverbSend = 0.0f;   // 0..1 (SPACE knob)
    float delaySend  = 0.0f;   // 0..1 (FX MIX knob)
    int   chokeGroup = 0;      // 0 = none
    bool  mute = false;

    bool hasSample() const noexcept { return sample.getNumSamples() > 0; }
};

//==============================================================================
/** Zavalishin/Simper TPT state-variable filter (per voice, mono). */
struct SvfTPT
{
    float g = 0, k = 1, a1 = 0, a2 = 0, a3 = 0, ic1 = 0, ic2 = 0;
    int type = 0;

    void set (double cutoffHz, double reso01, double sr)
    {
        const double fc = juce::jlimit (30.0, sr * 0.49, cutoffHz);
        g = (float) std::tan (juce::MathConstants<double>::pi * fc / sr);
        const double Q = juce::jlimit (0.5, 12.0, 0.5 + reso01 * 11.5);
        k = (float) (1.0 / Q);
        const double a1d = 1.0 / (1.0 + (double) g * ((double) g + (double) k));
        a1 = (float) a1d; a2 = (float) (g * a1d); a3 = (float) (g * a2);
    }
    void reset() { ic1 = ic2 = 0; }
    inline float process (float v0)
    {
        const float v3 = v0 - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2 * v1 - ic1;
        ic2 = 2 * v2 - ic2;
        if (type == 0) return v2;                 // LP
        if (type == 1) return v0 - k * v1 - v2;   // HP
        return v1;                                // BP
    }
};

//==============================================================================
/** Self-freeing one-shot amp envelope (A / D / S-hold / R). */
struct AmpEnv
{
    enum class St { Idle, Atk, Dec, Sus, Rel, Done };
    St st = St::Idle;
    double sr = 44100.0;
    float level = 0.0f, sustain = 0.0f, atkInc = 1.0f, decCoef = 0.0f, relCoef = 0.0f;

    void setSampleRate (double s) { sr = s; }

    void start (float a, float d, float s, float r)
    {
        sustain = juce::jlimit (0.0f, 1.0f, s);
        atkInc  = a > 1.0e-4f ? (float) (1.0 / (a * sr)) : 1.0f;
        decCoef = d > 1.0e-4f ? (float) std::exp (-1.0 / (d * sr * 0.3)) : 0.0f;
        relCoef = r > 1.0e-4f ? (float) std::exp (-1.0 / (r * sr * 0.3)) : 0.0f;
        level = 0.0f; st = St::Atk;
    }
    void noteOff() { if (st != St::Idle && st != St::Done) st = St::Rel; }

    inline float next()
    {
        switch (st)
        {
            case St::Atk: level += atkInc; if (level >= 1.0f) { level = 1.0f; st = St::Dec; } break;
            case St::Dec:
                level = sustain + (level - sustain) * decCoef;
                if (level <= sustain + 1.0e-4f)
                {
                    if (sustain <= 1.0e-4f) { level = 0.0f; st = St::Done; }
                    else                    { level = sustain; st = St::Sus; }
                }
                break;
            case St::Sus: level = sustain; break;
            case St::Rel: level *= relCoef; if (level <= 1.0e-4f) { level = 0.0f; st = St::Done; } break;
            default: level = 0.0f; break;
        }
        return level;
    }
    bool active() const { return st != St::Idle && st != St::Done; }
};

//==============================================================================
/** One playback voice: reads a pad's sample with pitch/glide, filters, envelopes. */
struct PadVoice
{
    int   pad = -1;
    int   choke = 0;
    juce::uint32 startOrder = 0;   // for voice stealing (oldest first)

    const juce::AudioBuffer<float>* src = nullptr;
    double readPos = 0.0, end = 0.0;
    double baseRate = 1.0;         // source samples per output sample (pitch)
    float  velGain = 1.0f, gain = 1.0f, lGain = 1.0f, rGain = 1.0f, drive = 0.0f;
    float  revSend = 0.0f, delSend = 0.0f;
    bool   reverse = false;

    // glide (downward pitch env)
    float  glideDepth = 0.0f, glideEnv = 1.0f, glideCoef = 0.0f;

    AmpEnv env;
    SvfTPT filt;
    float  chokeGain = 1.0f, chokeStep = 0.0f;

    bool active() const { return pad >= 0; }
    void kill() { pad = -1; }
};

//==============================================================================
class DrumEngine
{
public:
    DrumEngine();

    void prepare (double sampleRate, int blockSize);
    void reset();

    /** Trigger a pad. sampleOffset = delay within current block (0 = block start).
        extraSemis = one-shot pitch offset on top of the pad tuning (e.g. touch ribbon). */
    void noteOn (int padIndex, float velocity, int sampleOffset = 0, float extraSemis = 0.0f);

    /** Render active voices into dry + parallel reverb/delay send buffers (all stereo, pre-cleared). */
    void process (juce::AudioBuffer<float>& dry,
                  juce::AudioBuffer<float>& revSend,
                  juce::AudioBuffer<float>& delSend,
                  int numSamples);

    // --- kit management ---
    void loadDefaultKit();
    void loadFactoryPad (int padIndex);   // embedded sample if available, else procedural
    void resetPadToDefault (int padIndex);
    bool loadSampleIntoPad (int padIndex, const juce::File& file);

    PadSettings&       pad (int i)       { return pads[(size_t) juce::jlimit (0, kNumPads - 1, i)]; }
    const PadSettings& pad (int i) const { return pads[(size_t) juce::jlimit (0, kNumPads - 1, i)]; }

    // --- metering (UI) ---
    float getPadLevel (int i) const { return padLevel[(size_t) juce::jlimit (0, kNumPads - 1, i)].load(); }
    void  decayMeters();

    /** Global pitch offset in semitones (footer PITCH wheel), applied at note-on. */
    void setGlobalPitchSemis (float s) { globalPitch.store (s); }

    // --- state ---
    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree&);

    double getSampleRate() const { return sr; }

private:
    void startVoice (PadVoice& v, int padIndex, float velocity, int sampleOffset, float extraSemis);
    PadVoice& allocateVoice();

    std::array<PadSettings, kNumPads> pads;
    std::array<PadVoice, kNumVoices>  voices;
    std::array<int, kNumVoices>       voiceDelay {};   // pre-trigger sample delay
    std::array<std::atomic<float>, kNumPads> padLevel {};

    double sr = 44100.0;
    juce::uint32 orderCounter = 0;
    std::atomic<float> globalPitch { 0.0f };
    juce::AudioFormatManager formatManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumEngine)
};
} // namespace mpc
