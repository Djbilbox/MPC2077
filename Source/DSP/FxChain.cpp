#include "FxChain.h"

using namespace mpc;

void FxChain::prepare (double sampleRate, int /*blockSize*/)
{
    sr = sampleRate;

    delayLen = juce::jmax (1, (int) (sr * 1.5));       // up to 1.5 s
    delayBuf.setSize (2, delayLen); delayBuf.clear();
    delayWrite = 0;

    glitchLen = juce::jmax (1, (int) (sr * 1.2));       // enough for slow-tempo 1/8 grains
    glitchBuf.setSize (2, glitchLen); glitchBuf.clear();
    glitchWrite = 0; grainPhase = 0.0; grainStart = 0; repeatActive = false;

    crushHoldL = crushHoldR = 0.0f; crushCounter = 0;
    wahL.reset(); wahR.reset(); wahPhase = 0.0;

    reverb.setSampleRate (sr);
    juce::Reverb::Parameters rp;
    rp.roomSize = 0.78f; rp.damping = 0.35f; rp.width = 1.0f;
    rp.wetLevel = 1.0f;  rp.dryLevel = 0.0f;   // pure wet (we mix manually)
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
}

void FxChain::reset()
{
    delayBuf.clear(); glitchBuf.clear();
    delayWrite = glitchWrite = 0; grainPhase = 0.0; grainStart = 0; repeatActive = false;
    crushHoldL = crushHoldR = 0.0f; crushCounter = 0;
    wahL.reset(); wahR.reset(); wahPhase = 0.0;
    reverb.reset();
}

//==============================================================================
void FxChain::process (juce::AudioBuffer<float>& dry,
                       juce::AudioBuffer<float>& revSend,
                       juce::AudioBuffer<float>& delSend,
                       const Params& p)
{
    const int n = dry.getNumSamples();
    auto* L = dry.getWritePointer (0);
    auto* R = dry.getWritePointer (1);
    auto* rsL = revSend.getReadPointer (0); auto* rsR = revSend.getReadPointer (1);
    auto* dsL = delSend.getReadPointer (0); auto* dsR = delSend.getReadPointer (1);

    const double bpm = juce::jlimit (20.0, 300.0, p.bpm);

    //--------------------------------------------------------------- BITCRUSH
    if (p.crush > 0.001f)
    {
        const int   hold = 1 + (int) (p.crush * p.crush * 24.0f);           // sample&hold factor
        const int   bits = juce::jlimit (2, 16, (int) std::round (16.0f - p.crush * 13.0f));
        const float q = std::pow (2.0f, (float) bits);
        for (int i = 0; i < n; ++i)
        {
            if (crushCounter <= 0) { crushHoldL = L[i]; crushHoldR = R[i]; crushCounter = hold; }
            --crushCounter;
            const float cL = std::round (crushHoldL * q) / q;
            const float cR = std::round (crushHoldR * q) / q;
            L[i] += (cL - L[i]) * p.crush;
            R[i] += (cR - R[i]) * p.crush;
        }
    }

    //-------------------------------------------------------------- CYBER WAH
    if (p.cyber > 0.001f)
    {
        const double wahRate = 0.4 + p.cyber * 5.0;              // Hz
        const double inc = 2.0 * juce::MathConstants<double>::pi * wahRate / sr;
        const float  drv = 1.0f + p.cyber * 6.0f;
        for (int i = 0; i < n; ++i)
        {
            wahPhase += inc; if (wahPhase > 2.0 * juce::MathConstants<double>::pi) wahPhase -= 2.0 * juce::MathConstants<double>::pi;
            const double lfo = 0.5 + 0.5 * std::sin (wahPhase);
            const double fc = 300.0 + lfo * 3200.0;
            const double Qv = 2.0 + p.cyber * 6.0;
            wahL.set (fc, Qv, sr); wahR.set (fc, Qv, sr);
            float wl = std::tanh (wahL.bp (L[i]) * drv);
            float wr = std::tanh (wahR.bp (R[i]) * drv);
            L[i] += (wl - L[i]) * p.cyber;
            R[i] += (wr - R[i]) * p.cyber;
        }
    }

    //---------------------------------------------------------------- GLITCH
    if (p.glitch > 0.001f)
    {
        const double grainLen = juce::jlimit (256.0, (double) glitchLen,
                                              (60.0 / bpm) * 0.25 * sr);   // 1/16 note
        const float gateDepth = p.glitch;
        for (int i = 0; i < n; ++i)
        {
            // write live signal into ring
            glitchBuf.setSample (0, glitchWrite, L[i]);
            glitchBuf.setSample (1, glitchWrite, R[i]);

            // grain boundary?
            if (grainPhase >= grainLen)
            {
                grainPhase -= grainLen;
                repeatActive = (rng.nextFloat() < p.glitch * 0.85f);
                grainStart = glitchWrite;   // freeze this grain's start
            }

            float gl = L[i], gr = R[i];
            if (repeatActive)
            {
                int idx = grainStart + (int) grainPhase;
                idx %= glitchLen; if (idx < 0) idx += glitchLen;
                gl = glitchBuf.getSample (0, idx);
                gr = glitchBuf.getSample (1, idx);
            }

            // rhythmic gate (square) inside the grain for extra chop
            const float gate = ((grainPhase / grainLen) < 0.5) ? 1.0f : (1.0f - gateDepth * 0.9f);
            gl *= gate; gr *= gate;

            L[i] += (gl - L[i]) * p.glitch;
            R[i] += (gr - R[i]) * p.glitch;

            grainPhase += 1.0;
            if (++glitchWrite >= glitchLen) glitchWrite = 0;
        }
    }

    //----------------------------------------------------------------- DELAY
    // always run so per-pad delaySends work even if the master slider is at 0
    {
        const int dTime = juce::jlimit (1, delayLen - 1, (int) ((60.0 / bpm) * 0.5 * sr)); // 1/8 note
        const float fb = 0.36f;
        const float send = p.delay;
        auto* dbL = delayBuf.getWritePointer (0);
        auto* dbR = delayBuf.getWritePointer (1);
        for (int i = 0; i < n; ++i)
        {
            int rd = delayWrite - dTime; if (rd < 0) rd += delayLen;
            const float wetL = dbL[rd];
            const float wetR = dbR[rd];

            const float inL = dsL[i] + L[i] * send;      // ping-pong: L in -> R line
            const float inR = dsR[i] + R[i] * send;
            dbL[delayWrite] = inR + wetR * fb;
            dbR[delayWrite] = inL + wetL * fb;
            if (++delayWrite >= delayLen) delayWrite = 0;

            L[i] += wetL;
            R[i] += wetR;
        }
    }

    //---------------------------------------------------------------- REVERB
    {
        // build reverb input into the (already consumed) send buffers, then process wet
        auto* riL = revSend.getWritePointer (0);
        auto* riR = revSend.getWritePointer (1);
        const float send = p.reverb;
        bool any = send > 0.001f;
        for (int i = 0; i < n; ++i)
        {
            riL[i] = rsL[i] + L[i] * send;
            riR[i] = rsR[i] + R[i] * send;
            if (! any && (std::abs (riL[i]) > 1.0e-6f || std::abs (riR[i]) > 1.0e-6f)) any = true;
        }
        if (any)
        {
            reverb.processStereo (riL, riR, n);
            for (int i = 0; i < n; ++i) { L[i] += riL[i]; R[i] += riR[i]; }
        }
    }

    //--------------------------------------------------- master safety soft-clip
    for (int i = 0; i < n; ++i)
    {
        L[i] = std::tanh (L[i] * 0.9f) / 0.9f * 0.94f;
        R[i] = std::tanh (R[i] * 0.9f) / 0.9f * 0.94f;
    }
}
