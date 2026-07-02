#include "DrumSynth.h"

using namespace mpc;

namespace
{
    constexpr double kTwoPi = 6.283185307179586476925;

    // --- small helpers ------------------------------------------------------
    inline float expEnv (double t, double decay)                 // 1 -> 0
    {
        return (float) std::exp (-t / juce::jmax (1.0e-4, decay));
    }

    inline float attackShape (double t, double atk)              // 0 -> 1
    {
        if (atk <= 1.0e-5) return 1.0f;
        return (float) juce::jlimit (0.0, 1.0, t / atk);
    }

    // one-pole low-pass state helper
    struct OnePole
    {
        float z = 0.0f, a = 0.1f;
        void setCutoff (double cutoffHz, double sr)
        {
            const double x = std::exp (-kTwoPi * juce::jlimit (10.0, sr * 0.49, cutoffHz) / sr);
            a = (float) (1.0 - x);
        }
        inline float lp (float in) { z += a * (in - z); return z; }
        inline float hp (float in) { return in - lp (in); }
    };

    void normalise (juce::AudioBuffer<float>& b, float peak = 0.98f)
    {
        auto mag = b.getMagnitude (0, b.getNumSamples());
        if (mag > 1.0e-6f)
            b.applyGain (peak / mag);
    }

    void fadeOut (juce::AudioBuffer<float>& b, int fadeSamples)
    {
        auto n = b.getNumSamples();
        fadeSamples = juce::jmin (fadeSamples, n);
        auto* d = b.getWritePointer (0);
        for (int i = 0; i < fadeSamples; ++i)
        {
            const float g = (float) (fadeSamples - i) / (float) fadeSamples;
            d[n - fadeSamples + i] *= g;
        }
    }
}

//==============================================================================
juce::String DrumSynth::defaultName (int pad)
{
    static const char* names[kNumPads] = {
        "KICK", "SUB KICK", "SNARE", "CLAP",
        "CL HAT", "OP HAT", "LOW TOM", "MID TOM",
        "HI TOM", "RIM", "CRASH", "RIDE",
        "ZAP", "COWBELL", "LASER", "NOISE"
    };
    return juce::String (names[juce::jlimit (0, kNumPads - 1, pad)]);
}

int DrumSynth::defaultChoke (int pad)
{
    // closed + open hat choke each other (group 1)
    if (pad == 4 || pad == 5) return 1;
    return 0;
}

//==============================================================================
void DrumSynth::renderKit (std::array<juce::AudioBuffer<float>, kNumPads>& out, double sr)
{
    for (int i = 0; i < kNumPads; ++i)
        renderPad (i, out[(size_t) i], sr);
}

void DrumSynth::renderPad (int pad, juce::AudioBuffer<float>& out, double sr)
{
    juce::Random rng ((juce::int64) (pad * 2654435761u + 12345u));

    auto seconds = [sr] (double s) { return juce::jmax (1, (int) std::ceil (s * sr)); };
    const double dt = 1.0 / sr;

    switch (pad)
    {
        //------------------------------------------------------------------ KICK
        case 0:
        {
            const double len = 0.55;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            double phase = 0.0;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                const double f = 48.0 + 90.0 * expEnv (t, 0.045);       // pitch drop
                phase += kTwoPi * f * dt;
                float body = (float) std::sin (phase) * expEnv (t, 0.34);
                float click = (float) std::sin (kTwoPi * 1200.0 * t) * expEnv (t, 0.004) * 0.5f;
                d[i] = std::tanh ((body + click) * 1.4f);
            }
            break;
        }
        //-------------------------------------------------------------- SUB KICK
        case 1:
        {
            const double len = 0.8;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            double phase = 0.0;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                const double f = 38.0 + 55.0 * expEnv (t, 0.07);
                phase += kTwoPi * f * dt;
                d[i] = (float) std::sin (phase) * expEnv (t, 0.55);
            }
            break;
        }
        //---------------------------------------------------------------- SNARE
        case 2:
        {
            const double len = 0.28;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            OnePole hp; hp.setCutoff (1500.0, sr);
            OnePole lp; lp.setCutoff (7500.0, sr);
            double p1 = 0.0, p2 = 0.0;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                p1 += kTwoPi * 185.0 * dt; p2 += kTwoPi * 330.0 * dt;
                float tone = (float) (std::sin (p1) + 0.6 * std::sin (p2)) * expEnv (t, 0.09) * 0.6f;
                float n = (rng.nextFloat() * 2.0f - 1.0f);
                n = lp.lp (hp.hp (n)) * expEnv (t, 0.10) * 0.9f;
                d[i] = std::tanh ((tone + n) * 1.2f);
            }
            break;
        }
        //----------------------------------------------------------------- CLAP
        case 3:
        {
            const double len = 0.3;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            OnePole hp; hp.setCutoff (1100.0, sr);
            OnePole lp; lp.setCutoff (6000.0, sr);
            const double bursts[4] = { 0.0, 0.011, 0.022, 0.033 };
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                float amp = 0.0f;
                for (double b : bursts)
                    if (t >= b) amp += expEnv (t - b, 0.012);
                amp += expEnv (t, 0.16) * 0.7f;                    // tail
                float n = lp.lp (hp.hp (rng.nextFloat() * 2.0f - 1.0f));
                d[i] = n * juce::jmin (1.0f, amp) * 0.9f;
            }
            break;
        }
        //-------------------------------------------------------- CLOSED / OPEN HAT
        case 4:
        case 5:
        {
            const double len = (pad == 4) ? 0.06 : 0.32;
            const double dec = (pad == 4) ? 0.028 : 0.20;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            OnePole hp; hp.setCutoff (7000.0, sr);
            // metallic bank of detuned squares
            const double fs[6] = { 320, 540, 800, 1180, 1600, 2200 };
            double ph[6] = {};
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                float metal = 0.0f;
                for (int k = 0; k < 6; ++k)
                {
                    ph[k] += kTwoPi * fs[k] * 6.0 * dt;
                    metal += (std::sin (ph[k]) > 0 ? 1.0f : -1.0f);
                }
                metal /= 6.0f;
                float n = rng.nextFloat() * 2.0f - 1.0f;
                d[i] = hp.hp (0.6f * metal + 0.4f * n) * expEnv (t, dec);
            }
            break;
        }
        //--------------------------------------------------------- TOMS (low/mid/hi)
        case 6: case 7: case 8:
        {
            const double baseF = (pad == 6) ? 110.0 : (pad == 7) ? 165.0 : 240.0;
            const double len = 0.4;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            double phase = 0.0;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                const double f = baseF * (1.0 + 0.6 * expEnv (t, 0.05));
                phase += kTwoPi * f * dt;
                float n = (rng.nextFloat() * 2.0f - 1.0f) * expEnv (t, 0.01) * 0.15f;
                d[i] = ((float) std::sin (phase) + n) * expEnv (t, 0.22);
            }
            break;
        }
        //------------------------------------------------------------------- RIM
        case 9:
        {
            const double len = 0.06;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            double p1 = 0.0, p2 = 0.0;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                p1 += kTwoPi * 1700.0 * dt; p2 += kTwoPi * 2500.0 * dt;
                d[i] = (float) (std::sin (p1) + std::sin (p2)) * 0.5f * expEnv (t, 0.012);
            }
            break;
        }
        //--------------------------------------------------------- CRASH / RIDE
        case 10:
        case 11:
        {
            const double len = (pad == 10) ? 1.2 : 0.6;
            const double dec = (pad == 10) ? 0.7 : 0.35;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            OnePole hp; hp.setCutoff (5000.0, sr);
            const double fs[8] = { 410, 600, 830, 1150, 1600, 2100, 2900, 3800 };
            double ph[8] = {};
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                float metal = 0.0f;
                for (int k = 0; k < 8; ++k) { ph[k] += kTwoPi * fs[k] * dt; metal += (std::sin (ph[k]) > 0 ? 1.0f : -1.0f); }
                metal /= 8.0f;
                float n = rng.nextFloat() * 2.0f - 1.0f;
                float atk = attackShape (t, 0.004);
                d[i] = hp.hp (0.5f * metal + 0.5f * n) * expEnv (t, dec) * atk;
            }
            break;
        }
        //------------------------------------------------------------------- ZAP
        case 12:
        {
            const double len = 0.18;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            double phase = 0.0;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                const double f = 900.0 * expEnv (t, 0.05) + 120.0;
                phase += kTwoPi * f * dt;
                d[i] = (float) std::sin (phase) * expEnv (t, 0.08);
            }
            break;
        }
        //--------------------------------------------------------------- COWBELL
        case 13:
        {
            const double len = 0.35;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            double p1 = 0.0, p2 = 0.0;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                p1 += kTwoPi * 540.0 * dt; p2 += kTwoPi * 800.0 * dt;
                float sq = (std::sin (p1) > 0 ? 1.0f : -1.0f) + (std::sin (p2) > 0 ? 1.0f : -1.0f);
                d[i] = sq * 0.4f * expEnv (t, 0.12);
            }
            break;
        }
        //----------------------------------------------------------------- LASER
        case 14:
        {
            const double len = 0.35;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            double phase = 0.0;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                const double f = 2400.0 * expEnv (t, 0.12) + 90.0;
                phase += kTwoPi * f * dt;
                float saw = (float) (2.0 * (phase / kTwoPi - std::floor (phase / kTwoPi + 0.5)));
                d[i] = saw * expEnv (t, 0.16) * 0.8f;
            }
            break;
        }
        //----------------------------------------------------------- NOISE HIT
        default:
        {
            const double len = 0.22;
            out.setSize (1, seconds (len)); out.clear();
            auto* d = out.getWritePointer (0);
            OnePole lp;
            for (int i = 0; i < out.getNumSamples(); ++i)
            {
                const double t = i * dt;
                lp.setCutoff (400.0 + 8000.0 * expEnv (t, 0.05), sr);
                float n = lp.lp (rng.nextFloat() * 2.0f - 1.0f);
                d[i] = n * expEnv (t, 0.1);
            }
            break;
        }
    }

    normalise (out);
    fadeOut (out, juce::jmin (64, out.getNumSamples()));
}
