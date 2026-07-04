#include "DrumEngine.h"
#include "DrumSynth.h"

#if MPC_HAS_BINARY_DATA
 #include "BinaryData.h"
#endif

using namespace mpc;

namespace
{
#if MPC_HAS_BINARY_DATA
    struct Emb { const char* data; int size; };
    // pad index -> embedded factory WAV (generic names in assets/samples/)
    Emb embeddedForPad (int i)
    {
        switch (i)
        {
            case 0:  return { BinaryData::kick_wav,    BinaryData::kick_wavSize };
            case 1:  return { BinaryData::sub_wav,     BinaryData::sub_wavSize };
            case 2:  return { BinaryData::snare_wav,   BinaryData::snare_wavSize };
            case 3:  return { BinaryData::clap_wav,    BinaryData::clap_wavSize };
            case 4:  return { BinaryData::clhat_wav,   BinaryData::clhat_wavSize };
            case 5:  return { BinaryData::ophat_wav,   BinaryData::ophat_wavSize };
            case 6:  return { BinaryData::lowtom_wav,  BinaryData::lowtom_wavSize };
            case 7:  return { BinaryData::midtom_wav,  BinaryData::midtom_wavSize };
            case 8:  return { BinaryData::hitom_wav,   BinaryData::hitom_wavSize };
            case 9:  return { BinaryData::rim_wav,     BinaryData::rim_wavSize };
            case 10: return { BinaryData::crash_wav,   BinaryData::crash_wavSize };
            case 11: return { BinaryData::ride_wav,    BinaryData::ride_wavSize };
            case 12: return { BinaryData::clav_wav,    BinaryData::clav_wavSize };
            case 13: return { BinaryData::cowbell_wav, BinaryData::cowbell_wavSize };
            case 14: return { BinaryData::conga_wav,   BinaryData::conga_wavSize };
            default: return { BinaryData::snap_wav,    BinaryData::snap_wavSize };
        }
    }
#endif
}

//==============================================================================
DrumEngine::DrumEngine()
{
    formatManager.registerBasicFormats();
    loadDefaultKit();
    for (auto& l : padLevel) l.store (0.0f);
}

void DrumEngine::prepare (double sampleRate, int /*blockSize*/)
{
    const bool rateChanged = ! juce::approximatelyEqual (sampleRate, sr);
    sr = sampleRate;
    for (auto& v : voices) { v.env.setSampleRate (sr); v.kill(); }

    // Re-render only procedural pads at the new host rate (embedded/user samples
    // carry their own sourceRate and are resampled on playback).
    if (rateChanged)
        for (int i = 0; i < kNumPads; ++i)
            if (pads[(size_t) i].isProcedural)
                loadFactoryPad (i);
}

void DrumEngine::reset()
{
    for (auto& v : voices) { v.kill(); v.filt.reset(); }
    for (auto& l : padLevel) l.store (0.0f);
}

//==============================================================================
void DrumEngine::loadDefaultKit()
{
    for (int i = 0; i < kNumPads; ++i)
        loadFactoryPad (i);
}

void DrumEngine::loadFactoryPad (int padIndex)
{
    const int i = juce::jlimit (0, kNumPads - 1, padIndex);
    auto& p = pads[(size_t) i];
    p.name        = DrumSynth::defaultName (i);
    p.chokeGroup  = DrumSynth::defaultChoke (i);
    p.sampleName  = {};
    p.filePath    = {};
    p.isUserSample = false;

#if MPC_HAS_BINARY_DATA
    auto e = embeddedForPad (i);
    if (e.data != nullptr && e.size > 0)
    {
        std::unique_ptr<juce::AudioFormatReader> r (
            formatManager.createReaderFor (std::make_unique<juce::MemoryInputStream> (e.data, (size_t) e.size, false)));
        if (r != nullptr && r->lengthInSamples > 0)
        {
            const int len   = (int) juce::jmin ((juce::int64) (sr * 10.0), r->lengthInSamples);
            const int chans = (int) juce::jmin (2u, r->numChannels);
            p.sample.setSize (chans, len);
            r->read (&p.sample, 0, len, 0, true, chans > 1);
            p.sourceRate  = r->sampleRate > 0 ? r->sampleRate : sr;
            p.isProcedural = false;
            return;
        }
    }
#endif

    // procedural fallback (rate-dependent)
    DrumSynth::renderPad (i, p.sample, sr);
    p.sourceRate   = sr;
    p.isProcedural = true;
}

void DrumEngine::resetPadToDefault (int padIndex)
{
    loadFactoryPad (padIndex);
}

bool DrumEngine::loadSampleIntoPad (int padIndex, const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    if (reader == nullptr || reader->lengthInSamples <= 0)
        return false;

    const int len = (int) juce::jmin ((juce::int64) (60 * (juce::int64) reader->sampleRate), // cap 60s
                                      reader->lengthInSamples);
    const int chans = (int) juce::jmin (2u, reader->numChannels);

    juce::AudioBuffer<float> buf (chans, len);
    reader->read (&buf, 0, len, 0, true, chans > 1);

    auto& p = pads[(size_t) juce::jlimit (0, kNumPads - 1, padIndex)];
    p.sample = std::move (buf);
    p.sourceRate = reader->sampleRate > 0 ? reader->sampleRate : sr;
    p.isUserSample = true;
    p.isProcedural = false;
    p.filePath = file.getFullPathName();
    p.sampleName = file.getFileName();
    p.name = file.getFileNameWithoutExtension().toUpperCase().substring (0, 10);
    return true;
}

//==============================================================================
PadVoice& DrumEngine::allocateVoice()
{
    // free slot first
    for (auto& v : voices)
        if (! v.active())
            return v;

    // else steal the oldest
    PadVoice* oldest = &voices[0];
    for (auto& v : voices)
        if (v.startOrder < oldest->startOrder)
            oldest = &v;
    return *oldest;
}

void DrumEngine::noteOn (int padIndex, float velocity, int sampleOffset, float extraSemis)
{
    if (padIndex < 0 || padIndex >= kNumPads) return;
    auto& p = pads[(size_t) padIndex];
    if (p.mute || ! p.hasSample()) return;

    // choke group: fast-fade voices in the same non-zero group
    if (p.chokeGroup != 0)
        for (auto& v : voices)
            if (v.active() && v.choke == p.chokeGroup && v.chokeStep <= 0.0f)
                v.chokeStep = 1.0f / juce::jmax (1.0f, (float) (0.004 * sr));

    startVoice (allocateVoice(), padIndex, velocity, sampleOffset, extraSemis);
}

void DrumEngine::startVoice (PadVoice& v, int padIndex, float velocity, int sampleOffset, float extraSemis)
{
    const auto& p = pads[(size_t) padIndex];

    v.pad        = padIndex;
    v.choke      = p.chokeGroup;
    v.startOrder = ++orderCounter;
    v.src        = &p.sample;
    v.end        = (double) p.sample.getNumSamples();
    v.reverse    = p.reverse;
    v.readPos    = juce::jlimit (0.0, v.end - 1.0, p.startPos * (v.end - 1.0));

    const double semis = p.pitchSemis + p.fineCents * 0.01 + globalPitch.load() + extraSemis;
    v.baseRate   = (p.sourceRate / sr) * std::pow (2.0, semis / 12.0);

    v.velGain    = juce::jlimit (0.0f, 1.0f, velocity);
    v.gain       = p.gain;
    v.drive      = p.drive;
    v.revSend    = p.reverbSend;
    v.delSend    = p.delaySend;

    const float pan = juce::jlimit (-1.0f, 1.0f, p.pan);
    v.lGain = std::cos ((pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi) * 1.4142f;
    v.rGain = std::sin ((pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi) * 1.4142f;

    v.glideDepth = p.glide;
    v.glideEnv   = 1.0f;
    const double glideTime = 0.05 + p.glide * 0.25;
    v.glideCoef  = (float) std::exp (-1.0 / (glideTime * sr));

    v.chokeGain = 1.0f;
    v.chokeStep = 0.0f;

    v.filt.type = p.filterType;
    const double cutHz = 30.0 * std::pow (18000.0 / 30.0, juce::jlimit (0.0f, 1.0f, p.cutoff));
    v.filt.set (cutHz, p.reso, sr);
    v.filt.reset();

    v.env.setSampleRate (sr);
    v.env.start (p.attack, p.decay, p.sustain, p.release);

    voiceDelay[(size_t) (&v - voices.data())] = juce::jmax (0, sampleOffset);
}

//==============================================================================
void DrumEngine::process (juce::AudioBuffer<float>& dry,
                          juce::AudioBuffer<float>& revSend,
                          juce::AudioBuffer<float>& delSend,
                          int numSamples)
{
    float peak[kNumPads] = {};

    auto* dryL = dry.getWritePointer (0);
    auto* dryR = dry.getWritePointer (1);
    auto* revL = revSend.getWritePointer (0);
    auto* revR = revSend.getWritePointer (1);
    auto* delL = delSend.getWritePointer (0);
    auto* delR = delSend.getWritePointer (1);

    for (size_t vi = 0; vi < voices.size(); ++vi)
    {
        auto& v = voices[vi];
        if (! v.active()) continue;

        int delay = voiceDelay[vi];
        if (delay >= numSamples) { voiceDelay[vi] = delay - numSamples; continue; }
        voiceDelay[vi] = 0;

        const auto* srcData = v.src;
        const int   srcCh   = srcData->getNumChannels();
        const int   srcLen  = srcData->getNumSamples();
        const float* s0 = srcData->getReadPointer (0);
        const float* s1 = srcCh > 1 ? srcData->getReadPointer (1) : s0;
        bool finished = false;

        for (int i = delay; i < numSamples; ++i)
        {
            if (v.readPos >= v.end - 1.0 || ! v.env.active()) { finished = true; break; }

            const double pos = v.reverse ? (v.end - 1.0 - v.readPos) : v.readPos;
            const int i0 = (int) pos;
            const int i1 = juce::jmin (srcLen - 1, i0 + 1);
            const float fr = (float) (pos - i0);
            const float sL = s0[i0] + (s0[i1] - s0[i0]) * fr;
            const float sR = s1[i0] + (s1[i1] - s1[i0]) * fr;

            v.glideEnv *= v.glideCoef;
            const double rate = v.baseRate * std::pow (2.0, (double) v.glideDepth * v.glideEnv);
            v.readPos += rate;

            float e = v.env.next();
            if (v.chokeStep > 0.0f)
            {
                v.chokeGain -= v.chokeStep;
                if (v.chokeGain <= 0.0f) { finished = true; break; }
                e *= v.chokeGain;
            }

            float s = 0.5f * (sL + sR) * v.velGain * v.gain * e;
            if (v.drive > 0.0f)
                s = std::tanh (s * (1.0f + v.drive * 8.0f)) / (1.0f + v.drive * 0.6f);
            s = v.filt.process (s);

            const float l = s * v.lGain;
            const float r = s * v.rGain;
            dryL[i] += l;  dryR[i] += r;
            revL[i] += l * v.revSend;  revR[i] += r * v.revSend;
            delL[i] += l * v.delSend;  delR[i] += r * v.delSend;

            const float a = std::abs (s);
            if (a > peak[v.pad]) peak[v.pad] = a;
        }

        if (finished) v.kill();
    }

    // meter update with VU-style decay
    for (int p = 0; p < kNumPads; ++p)
    {
        const float prev = padLevel[(size_t) p].load() * 0.80f;
        padLevel[(size_t) p].store (juce::jmax (prev, peak[p]));
    }
}

void DrumEngine::decayMeters()
{
    for (auto& l : padLevel) l.store (l.load() * 0.85f);
}

//==============================================================================
juce::ValueTree DrumEngine::toValueTree() const
{
    juce::ValueTree tree ("MPC_ENGINE");
    for (int i = 0; i < kNumPads; ++i)
    {
        const auto& p = pads[(size_t) i];
        juce::ValueTree pt ("PAD");
        pt.setProperty ("idx", i, nullptr);
        pt.setProperty ("name", p.name, nullptr);
        pt.setProperty ("isUser", p.isUserSample, nullptr);
        pt.setProperty ("path", p.filePath, nullptr);
        pt.setProperty ("pitch", p.pitchSemis, nullptr);
        pt.setProperty ("fine", p.fineCents, nullptr);
        pt.setProperty ("glide", p.glide, nullptr);
        pt.setProperty ("start", p.startPos, nullptr);
        pt.setProperty ("reverse", p.reverse, nullptr);
        pt.setProperty ("atk", p.attack, nullptr);
        pt.setProperty ("dec", p.decay, nullptr);
        pt.setProperty ("sus", p.sustain, nullptr);
        pt.setProperty ("rel", p.release, nullptr);
        pt.setProperty ("ftype", p.filterType, nullptr);
        pt.setProperty ("cut", p.cutoff, nullptr);
        pt.setProperty ("reso", p.reso, nullptr);
        pt.setProperty ("gain", p.gain, nullptr);
        pt.setProperty ("pan", p.pan, nullptr);
        pt.setProperty ("drive", p.drive, nullptr);
        pt.setProperty ("rsend", p.reverbSend, nullptr);
        pt.setProperty ("dsend", p.delaySend, nullptr);
        pt.setProperty ("choke", p.chokeGroup, nullptr);
        pt.setProperty ("mute", p.mute, nullptr);
        tree.appendChild (pt, nullptr);
    }
    return tree;
}

void DrumEngine::fromValueTree (const juce::ValueTree& tree)
{
    if (! tree.hasType ("MPC_ENGINE")) return;

    for (int c = 0; c < tree.getNumChildren(); ++c)
    {
        auto pt = tree.getChild (c);
        if (! pt.hasType ("PAD")) continue;
        const int i = (int) pt.getProperty ("idx", -1);
        if (i < 0 || i >= kNumPads) continue;

        auto& p = pads[(size_t) i];
        const bool isUser = (bool) pt.getProperty ("isUser", false);
        const juce::String path = pt.getProperty ("path", juce::String());

        if (isUser && path.isNotEmpty() && juce::File (path).existsAsFile())
        {
            loadSampleIntoPad (i, juce::File (path));
        }
        else if (isUser)
        {
            // file gone: keep params but fall back to procedural sound
            resetPadToDefault (i);
        }
        // (procedural pads keep their already-rendered sample)

        p.name        = pt.getProperty ("name", p.name);
        p.pitchSemis  = (float) pt.getProperty ("pitch", p.pitchSemis);
        p.fineCents   = (float) pt.getProperty ("fine", p.fineCents);
        p.glide       = (float) pt.getProperty ("glide", p.glide);
        p.startPos    = (float) pt.getProperty ("start", p.startPos);
        p.reverse     = (bool)  pt.getProperty ("reverse", p.reverse);
        p.attack      = (float) pt.getProperty ("atk", p.attack);
        p.decay       = (float) pt.getProperty ("dec", p.decay);
        p.sustain     = (float) pt.getProperty ("sus", p.sustain);
        p.release     = (float) pt.getProperty ("rel", p.release);
        p.filterType  = (int)   pt.getProperty ("ftype", p.filterType);
        p.cutoff      = (float) pt.getProperty ("cut", p.cutoff);
        p.reso        = (float) pt.getProperty ("reso", p.reso);
        p.gain        = (float) pt.getProperty ("gain", p.gain);
        p.pan         = (float) pt.getProperty ("pan", p.pan);
        p.drive       = (float) pt.getProperty ("drive", p.drive);
        p.reverbSend  = (float) pt.getProperty ("rsend", p.reverbSend);
        p.delaySend   = (float) pt.getProperty ("dsend", p.delaySend);
        p.chokeGroup  = (int)   pt.getProperty ("choke", p.chokeGroup);
        p.mute        = (bool)  pt.getProperty ("mute", p.mute);
    }
}
