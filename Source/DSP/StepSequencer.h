#pragma once

#include <JuceHeader.h>
#include "MpcConstants.h"

namespace mpc
{
/**
    Per-pad step sequencer. Host-transport synced (locks to ppqPosition) with a
    free-running internal clock fallback for the standalone. Produces sample-accurate
    pad triggers via a callback. Pure DSP — no UI.
*/
class StepSequencer
{
public:
    struct Transport
    {
        double bpm         = 120.0;
        double ppqPosition = 0.0;
        bool   isPlaying   = false;
        bool   valid       = false;
    };

    StepSequencer();

    void prepare (double sampleRate);
    void reset();

    // --- pattern editing (message thread) ---
    bool  getStep (int pad, int step) const;
    void  setStep (int pad, int step, bool on);
    void  toggleStep (int pad, int step);
    float getVel  (int pad, int step) const;
    void  setVel  (int pad, int step, float v);
    void  clearPad (int pad);
    void  clearAll();

    void setNumSteps (int n)      { numSteps.store (juce::jlimit (1, kMaxSteps, n)); }
    int  getNumSteps() const      { return numSteps.load(); }
    void setSwing (float s)       { swing.store (juce::jlimit (0.0f, 1.0f, s)); }
    void setHumanize (float h)    { humanize.store (juce::jlimit (0.0f, 1.0f, h)); }
    void setInternalTempo (double bpm) { internalBpm = juce::jlimit (20.0, 300.0, bpm); }
    void setInternalPlaying (bool p)   { internalPlaying.store (p); }
    bool isInternalPlaying() const     { return internalPlaying.load(); }

    int  getCurrentStep() const { return currentStep.load(); }
    bool isRunning() const      { return running.load(); }

    /** Advance the sequencer over one block, firing trigger(pad, sampleOffset, velocity). */
    template <typename TriggerFn>
    void process (int numSamples, const Transport& t, TriggerFn&& trigger)
    {
        const bool hostPlay = t.isPlaying && t.valid;
        const bool play = hostPlay || internalPlaying.load();
        running.store (play);
        if (! play) { currentStep.store (-1); return; }

        const double bpm = (t.valid && t.bpm > 1.0) ? t.bpm : internalBpm;
        const double stepsPerSample = (bpm / 60.0) * 4.0 / sr;   // 16ths per sample
        const int    n  = numSteps.load();
        const float  sw = swing.load() * 0.5f;                   // up to half a step
        const float  hum = humanize.load();

        const double p0 = hostPlay ? (t.ppqPosition * 4.0) : internalPhase;
        const double p1 = p0 + (double) numSamples * stepsPerSample;

        const int kStart = (int) std::floor (p0) - 1;
        const int kEnd   = (int) std::ceil  (p1) + 1;

        for (int k = kStart; k <= kEnd; ++k)
        {
            const int idx = ((k % n) + n) % n;
            const double pk = (double) k + ((k & 1) ? (double) sw : 0.0);
            if (pk < p0 || pk >= p1) continue;

            double pkh = pk;
            if (hum > 0.0f) pkh += (rng.nextFloat() * 2.0f - 1.0f) * hum * 0.12f;

            int off = (int) ((pkh - p0) / juce::jmax (1.0e-9, stepsPerSample) + 0.5);
            off = juce::jlimit (0, numSamples - 1, off);

            for (int pad = 0; pad < kNumPads; ++pad)
            {
                if (! steps[(size_t) (pad * kMaxSteps + idx)].load()) continue;
                float vel = vels[(size_t) (pad * kMaxSteps + idx)].load();
                if (hum > 0.0f) vel *= 1.0f - hum * 0.2f * rng.nextFloat();
                trigger (pad, off, juce::jlimit (0.0f, 1.0f, vel));
            }
        }

        currentStep.store ((((int) std::floor (p1) % n) + n) % n);
        internalPhase = p1;   // keep continuous for standalone / transport handoff
    }

    juce::ValueTree toValueTree() const;
    void fromValueTree (const juce::ValueTree&);

private:
    std::array<std::atomic<bool>,  kNumPads * kMaxSteps> steps;
    std::array<std::atomic<float>, kNumPads * kMaxSteps> vels;

    std::atomic<int>   numSteps { 16 };
    std::atomic<float> swing    { 0.0f };
    std::atomic<float> humanize { 0.0f };
    std::atomic<bool>  internalPlaying { false };
    std::atomic<bool>  running  { false };
    std::atomic<int>   currentStep { -1 };

    double sr = 44100.0;
    double internalBpm = 120.0;
    double internalPhase = 0.0;
    juce::Random rng;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepSequencer)
};
} // namespace mpc
