#pragma once

#include <JuceHeader.h>

namespace mpc
{
/**
    Master FX bus. Applies insert FX (bitcrush, cyber auto-wah, glitch beat-repeat)
    to the dry drum sum, plus send/return delay and reverb fed by both the global
    sliders and the per-pad send buffers. Pure DSP.
*/
class FxChain
{
public:
    struct Params
    {
        float reverb = 0.0f;  // 0..1  master reverb send
        float glitch = 0.0f;  // 0..1  beat-repeat / gate intensity
        float crush  = 0.0f;  // 0..1  bit + rate reduction
        float delay  = 0.0f;  // 0..1  master delay send
        float cyber  = 0.0f;  // 0..1  resonant auto-wah + drive
        double bpm   = 120.0;
    };

    void prepare (double sampleRate, int blockSize);
    void reset();

    /** Process in place: dry becomes the wet master; revSend/delSend are the per-pad sends. */
    void process (juce::AudioBuffer<float>& dry,
                  juce::AudioBuffer<float>& revSend,
                  juce::AudioBuffer<float>& delSend,
                  const Params& p);

private:
    double sr = 44100.0;

    // --- delay ---
    juce::AudioBuffer<float> delayBuf;
    int delayWrite = 0, delayLen = 0;

    // --- glitch beat-repeat ring ---
    juce::AudioBuffer<float> glitchBuf;
    int glitchWrite = 0, glitchLen = 0;
    double grainPhase = 0.0;
    int    grainStart = 0;
    bool   repeatActive = false;
    juce::Random rng;

    // --- bitcrush state ---
    float crushHoldL = 0.0f, crushHoldR = 0.0f;
    int   crushCounter = 0;

    // --- cyber auto-wah ---
    struct Svf { float g=0,k=1,a1=0,a2=0,a3=0,ic1=0,ic2=0;
        void set(double fc,double q,double s){ g=(float)std::tan(juce::MathConstants<double>::pi*juce::jlimit(30.0,s*0.49,fc)/s);
            k=(float)(1.0/juce::jlimit(0.5,12.0,q)); double a=1.0/(1.0+(double)g*((double)g+(double)k)); a1=(float)a; a2=(float)(g*a); a3=(float)(g*a2);}
        void reset(){ ic1=ic2=0; }
        float bp(float v0){ float v3=v0-ic2, v1=a1*ic1+a2*v3, v2=ic2+a2*ic1+a3*v3; ic1=2*v1-ic1; ic2=2*v2-ic2; return v1; } };
    Svf wahL, wahR;
    double wahPhase = 0.0;

    juce::Reverb reverb;
};
} // namespace mpc
