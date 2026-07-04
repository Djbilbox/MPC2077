#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstring>

using namespace mpc;

//==============================================================================
namespace
{
    const char* kModeChoices[] = { "CHOP", "SLICE", "BEAT", "SEQ", "GLITCH", "CYBER" };

    const char* kPresetNames[FactoryPresetCount] = {
        // 0-9
        "Night City 808", "Corpo Beat Chop", "Arasaka Slam", "Johnny Silverhand Groove",
        "Netrunner Sequence", "Ghost In The Grid", "Blade Runner Trap", "Neon Rain Bounce",
        "Cyber Monk Slice", "Zero Day Glitch",
        // 10-19
        "Tokyo Neon Dream", "Hacker's Pulse", "Synth Wave Drift", "Digital Samurai",
        "Pixel Noir Bounce", "Data Storm", "Chrome Sunset", "Retro Future Beat",
        "Glitch City Groove", "Acid Rain Loop",
        // 20-29
        "Velvet Thunder", "Neon Genesis", "Cyberpunk Stomp", "Electric Vice",
        "Synth Explosion", "Dark Matter Kick", "Neo Tokyo Drive", "Void Walker",
        "Neon Pulse", "Retro Synth Wave",
        // 30-39
        "Fever Dream", "Laser Beat", "Synth Breaker", "Cosmic Bounce",
        "Digital Heat", "Neon Lights", "Thunder Road", "Synth Killer",
        "Cyber Bounce", "Electric Surge",
        // 40-49
        "Pulsing Core", "Neon Frenzy", "Digital Breakdown", "Synth Prophet",
        "Glitch Funk", "Neon Carnival", "Synth Horizon", "Electric Monk",
        "Retro Digital", "Future Shock"
    };
}

//==============================================================================
MPC2077AudioProcessor::MPC2077AudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MPC2077", createLayout())
{
    for (auto& t : uiPadTrigger) t.store (-1.0f);
    for (auto& t : uiPadPitch)   t.store (0.0f);
    applyFactoryPreset (0);
}

MPC2077AudioProcessor::~MPC2077AudioProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout MPC2077AudioProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto pct = [] (const char* id, const char* name, float def)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name,
                                                       NormalisableRange<float> (0.0f, 1.0f), def);
    };

    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid::master, 1 }, "Master",
                                                       NormalisableRange<float> (0.0f, 1.5f), 1.0f));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { pid::mode, 1 }, "Mode",
                                                        StringArray (kModeChoices, 6), 2));
    layout.add (pct (pid::swing,    "Swing",    0.0f));
    layout.add (pct (pid::humanize, "Humanize", 0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { pid::pitchWheel, 1 }, "Pitch",
                                                       NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
    layout.add (pct (pid::modWheel, "Mod", 0.0f));

    layout.add (pct (pid::fxReverb, "Reverb", 0.25f));
    layout.add (pct (pid::fxGlitch, "Glitch", 0.0f));
    layout.add (pct (pid::fxCrush,  "Crush",  0.0f));
    layout.add (pct (pid::fxDelay,  "Delay",  0.15f));
    layout.add (pct (pid::fxCyber,  "Cyber FX", 0.0f));

    return layout;
}

//==============================================================================
bool MPC2077AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void MPC2077AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    sequencer.prepare (sampleRate);
    fx.prepare (sampleRate, samplesPerBlock);

    const int n = juce::jmax (16, samplesPerBlock);
    mixBuf.setSize (2, n);
    revBuf.setSize (2, n);
    delBuf.setSize (2, n);
}

//==============================================================================
void MPC2077AudioProcessor::triggerPadFromUI (int pad, float velocity, float extraSemis)
{
    if (pad >= 0 && pad < kNumPads)
    {
        uiPadPitch[(size_t) pad].store (extraSemis);
        uiPadTrigger[(size_t) pad].store (juce::jlimit (0.0f, 1.0f, velocity));
    }
}

//==============================================================================
void MPC2077AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();

    // --- read parameters ---
    auto pval = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };
    const float master = pval (pid::master);

    engine.setGlobalPitchSemis (pval (pid::pitchWheel) * 12.0f);
    sequencer.setSwing (pval (pid::swing));
    sequencer.setHumanize (pval (pid::humanize));

    FxChain::Params fxp;
    fxp.reverb = pval (pid::fxReverb);
    fxp.glitch = pval (pid::fxGlitch);
    fxp.crush  = pval (pid::fxCrush);
    fxp.delay  = pval (pid::fxDelay);
    fxp.cyber  = pval (pid::fxCyber);

    // --- transport ---
    StepSequencer::Transport tr;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto bpm = pos->getBpm())         tr.bpm = *bpm;
            if (auto ppq = pos->getPpqPosition()) tr.ppqPosition = *ppq;
            tr.isPlaying = pos->getIsPlaying();
            tr.valid = true;
        }
    }
    fxp.bpm = tr.bpm;

    // --- scratch buffers ---
    if (mixBuf.getNumSamples() < numSamples) { mixBuf.setSize (2, numSamples, false, false, true); revBuf.setSize (2, numSamples, false, false, true); delBuf.setSize (2, numSamples, false, false, true); }
    mixBuf.clear(); revBuf.clear(); delBuf.clear();

    // --- live UI pad hits (block start) ---
    for (int p = 0; p < kNumPads; ++p)
    {
        const float v = uiPadTrigger[(size_t) p].exchange (-1.0f);
        if (v >= 0.0f) engine.noteOn (p, v, 0, uiPadPitch[(size_t) p].load());
    }

    // --- incoming MIDI (note -> pad) ---
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.isNoteOn())
        {
            const int pad = m.getNoteNumber() - kBaseMidiNote;
            if (pad >= 0 && pad < kNumPads)
                engine.noteOn (pad, m.getFloatVelocity(), juce::jlimit (0, numSamples - 1, meta.samplePosition));
        }
    }

    // --- sequencer -> triggers ---
    sequencer.process (numSamples, tr, [this] (int pad, int off, float vel)
    {
        engine.noteOn (pad, vel, off);
    });

    // --- render + FX ---
    engine.process (mixBuf, revBuf, delBuf, numSamples);
    fx.process (mixBuf, revBuf, delBuf, fxp);
    mixBuf.applyGain (0, numSamples, master);

    // --- copy to output ---
    const int outCh = buffer.getNumChannels();
    if (outCh >= 2)
    {
        buffer.copyFrom (0, 0, mixBuf, 0, 0, numSamples);
        buffer.copyFrom (1, 0, mixBuf, 1, 0, numSamples);
        for (int c = 2; c < outCh; ++c) buffer.clear (c, 0, numSamples);
    }
    else if (outCh == 1)
    {
        buffer.copyFrom (0, 0, mixBuf, 0, 0, numSamples);
        buffer.addFrom  (0, 0, mixBuf, 1, 0, numSamples);
        buffer.applyGain (0, 0, numSamples, 0.5f);
    }

    // --- master meter ---
    float mag = 0.0f;
    for (int c = 0; c < juce::jmin (2, outCh); ++c)
        mag = juce::jmax (mag, buffer.getMagnitude (c, 0, numSamples));
    outputLevel.store (juce::jmax (outputLevel.load() * 0.7f, mag));

    midi.clear();
}

//==============================================================================
int MPC2077AudioProcessor::getNumPrograms() { return FactoryPresetCount; }

juce::String MPC2077AudioProcessor::factoryPresetName (int index)
{
    return juce::String (kPresetNames[juce::jlimit (0, FactoryPresetCount - 1, index)]);
}

const juce::String MPC2077AudioProcessor::getProgramName (int index) { return factoryPresetName (index); }

void MPC2077AudioProcessor::setCurrentProgram (int index)
{
    currentProgram = juce::jlimit (0, FactoryPresetCount - 1, index);
    applyFactoryPreset (currentProgram);
}

//==============================================================================
// Pattern helper: 'x'/'X' = hit, others = rest. Sets numSteps from string length.
static void setRow (StepSequencer& seq, int pad, const char* pat)
{
    const int len = (int) std::strlen (pat);
    for (int s = 0; s < kMaxSteps; ++s)
        seq.setStep (pad, s, s < len && (pat[s] == 'x' || pat[s] == 'X'));
}

void MPC2077AudioProcessor::applyFactoryPreset (int index)
{
    index = juce::jlimit (0, FactoryPresetCount - 1, index);
    currentProgram = index;

    engine.loadDefaultKit();
    sequencer.clearAll();
    sequencer.setNumSteps (16);

    auto setParam = [this] (const char* id, float v)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (apvts.getParameterRange (id).convertTo0to1 (v));
    };

    // pad indices: 0 KICK 1 SUBKICK 2 SNARE 3 CLAP 4 CLHAT 5 OPHAT 6-8 TOMS 9 RIM 10 CRASH 11 RIDE 12 ZAP 13 COWBELL 14 LASER 15 NOISE
    float swing = 0.0f, hum = 0.0f, mode = 2.0f;
    float rvb = 0.25f, gli = 0.0f, cru = 0.0f, dly = 0.15f, cyb = 0.0f;

    switch (index)
    {
        case 0: // Night City 808 — boom-bap 808
            setRow (sequencer, 0, "x.......x.......");
            setRow (sequencer, 1, "....x.......x...");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            rvb = 0.3f; dly = 0.2f; swing = 0.15f; break;
        case 1: // Corpo Beat Chop
            setRow (sequencer, 0, "x..x..x...x..x..");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            setRow (sequencer, 3, "........x.......");
            cru = 0.2f; swing = 0.25f; break;
        case 2: // Arasaka Slam — hard
            setRow (sequencer, 0, "x.....x.x.....x.");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 5, "..x...x...x...x.");
            cyb = 0.3f; rvb = 0.2f; mode = 5.0f; break;
        case 3: // Johnny Silverhand Groove — rock/funk
            setRow (sequencer, 0, "x...x..x.x...x..");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 4, "xxxxxxxxxxxxxxxx");
            swing = 0.35f; dly = 0.25f; break;
        case 4: // Netrunner Sequence — synced 16ths
            setRow (sequencer, 0, "x.......x.......");
            setRow (sequencer, 12, "x.x.x.x.x.x.x.x.");
            setRow (sequencer, 14, "..x...x...x...x.");
            mode = 3.0f; dly = 0.4f; rvb = 0.35f; break;
        case 5: // Ghost In The Grid — ambient
            setRow (sequencer, 1, "x.......x.......");
            setRow (sequencer, 5, "....x.......x...");
            setRow (sequencer, 14, "..x...x...x...x.");
            rvb = 0.6f; dly = 0.45f; cyb = 0.2f; break;
        case 6: // Blade Runner Trap — trap hats
            setRow (sequencer, 0, "x.....x...x.....");
            setRow (sequencer, 2, "........x.......");
            setRow (sequencer, 4, "xxxxxx.xxxxx.xxx");
            swing = 0.2f; rvb = 0.3f; break;
        case 7: // Neon Rain Bounce
            setRow (sequencer, 0, "x..x..x..x..x..x");
            setRow (sequencer, 3, "....x.......x...");
            setRow (sequencer, 5, "..x...x...x...x.");
            swing = 0.4f; dly = 0.3f; break;
        case 8: // Cyber Monk Slice
            setRow (sequencer, 0, "x...x...x...x...");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 13, "..x.x...x.x.x...");
            mode = 1.0f; cyb = 0.25f; break;
        case 9: // Zero Day Glitch
            setRow (sequencer, 0, "x...x.x.x...x.x.");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 15, "x.x.x.x.x.x.x.x.");
            gli = 0.55f; cru = 0.3f; mode = 4.0f; break;
        case 10: // Tokyo Neon Dream
            setRow (sequencer, 0, "x.x.x.x.x.x.x.x.");
            setRow (sequencer, 2, "..x.....x.x.....");
            setRow (sequencer, 5, "....x.......x...");
            rvb = 0.4f; dly = 0.3f; break;
        case 11: // Hacker's Pulse
            setRow (sequencer, 0, "x..x..x...x.x...");
            setRow (sequencer, 3, "....x.x.....x...");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            cru = 0.25f; gli = 0.3f; break;
        case 12: // Synth Wave Drift
            setRow (sequencer, 1, "x.......x.......");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 5, "..x...x...x...x.");
            dly = 0.35f; rvb = 0.45f; break;
        case 13: // Digital Samurai
            setRow (sequencer, 0, "x...x..x...x...x");
            setRow (sequencer, 2, ".x.x.x.x.x.x.x.x");
            setRow (sequencer, 4, "x.x.x.x.......");
            swing = 0.25f; break;
        case 14: // Pixel Noir Bounce
            setRow (sequencer, 0, "x..x..x..x..x..x");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 5, ".x.x.x.x.x.x.x.");
            swing = 0.35f; dly = 0.2f; break;
        case 15: // Data Storm
            setRow (sequencer, 0, "x.x...x.x.x...x.");
            setRow (sequencer, 3, "........x.......");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            gli = 0.4f; cru = 0.2f; break;
        case 16: // Chrome Sunset
            setRow (sequencer, 0, "x.....x.x.....x.");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 12, "..x...x...x...x.");
            rvb = 0.35f; dly = 0.25f; break;
        case 17: // Retro Future Beat
            setRow (sequencer, 0, "x..x..x...x..x..");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 4, "xxxxxxxxxxxxxxxx");
            swing = 0.2f; break;
        case 18: // Glitch City Groove
            setRow (sequencer, 0, "x...x...x...x...");
            setRow (sequencer, 2, ".x.x.x.x.x.x.x.");
            setRow (sequencer, 15, "x.x.x.x.x.x.x.x.");
            gli = 0.35f; cru = 0.35f; break;
        case 19: // Acid Rain Loop
            setRow (sequencer, 0, "x.......x.......");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 13, "xxxxxxxxxxxx....");
            dly = 0.4f; rvb = 0.3f; break;
        case 20: // Velvet Thunder
            setRow (sequencer, 0, "x.x.x...x.x.x...");
            setRow (sequencer, 2, "....x.x.....x.x.");
            setRow (sequencer, 5, "x.x.x.x.x.x.x.x.");
            rvb = 0.25f; swing = 0.15f; break;
        case 21: // Neon Genesis
            setRow (sequencer, 0, "x..x..x..x..x..x");
            setRow (sequencer, 3, "....x.......x...");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            mode = 2.0f; dly = 0.2f; break;
        case 22: // Cyberpunk Stomp
            setRow (sequencer, 0, "x...x...x...x...");
            setRow (sequencer, 2, "..x.x.x.x.x.x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            cyb = 0.25f; break;
        case 23: // Electric Vice
            setRow (sequencer, 0, "x.x.x.x.x.x.x.x.");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 3, ".x.x.x.x.x.x.x.");
            swing = 0.3f; break;
        case 24: // Synth Explosion
            setRow (sequencer, 0, "x...x...x...x...");
            setRow (sequencer, 2, ".x.x.x.x.x.x.x.");
            setRow (sequencer, 5, "x.x.x.x.x.x.x.x.");
            rvb = 0.5f; break;
        case 25: // Dark Matter Kick
            setRow (sequencer, 0, "x.......x.......");
            setRow (sequencer, 1, "..x...x...x...x.");
            setRow (sequencer, 2, "....x.......x...");
            gli = 0.2f; break;
        case 26: // Neo Tokyo Drive
            setRow (sequencer, 0, "x.x.x...x.x...x.");
            setRow (sequencer, 2, "....x.x.....x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            dly = 0.3f; break;
        case 27: // Void Walker
            setRow (sequencer, 0, "x...x.x.x...x.x.");
            setRow (sequencer, 3, "........x.......");
            setRow (sequencer, 5, "..x...x...x...x.");
            cyb = 0.3f; rvb = 0.4f; break;
        case 28: // Neon Pulse
            setRow (sequencer, 0, "x..x..x..x..x..x");
            setRow (sequencer, 2, ".x.x.x.x.x.x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            mode = 3.0f; break;
        case 29: // Retro Synth Wave
            setRow (sequencer, 0, "x.......x.......");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 13, "x.x.x.x.x.x.x.x.");
            rvb = 0.45f; dly = 0.35f; break;
        case 30: // Fever Dream
            setRow (sequencer, 0, "x.x...x.x.x...x.");
            setRow (sequencer, 2, "....x.x.....x.x.");
            setRow (sequencer, 5, "x.x.x.x.x.x.x.x.");
            swing = 0.25f; break;
        case 31: // Laser Beat
            setRow (sequencer, 0, "x...x...x...x...");
            setRow (sequencer, 3, ".x.x.x.x.x.x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            cyb = 0.2f; gli = 0.25f; break;
        case 32: // Synth Breaker
            setRow (sequencer, 0, "x..x..x...x..x..");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 4, "xxxxxxxxxxxxxxxx");
            cru = 0.2f; break;
        case 33: // Cosmic Bounce
            setRow (sequencer, 0, "x..x..x..x..x..x");
            setRow (sequencer, 3, "....x.......x...");
            setRow (sequencer, 5, "..x...x...x...x.");
            dly = 0.25f; rvb = 0.35f; break;
        case 34: // Digital Heat
            setRow (sequencer, 0, "x.x.x.x.x.x.x.x.");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 3, ".x.x.x.x.x.x.x.");
            gli = 0.3f; break;
        case 35: // Neon Lights
            setRow (sequencer, 0, "x...x...x...x...");
            setRow (sequencer, 2, ".x.x.x.x.x.x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            rvb = 0.3f; mode = 4.0f; break;
        case 36: // Thunder Road
            setRow (sequencer, 0, "x.......x.......");
            setRow (sequencer, 2, "....x.x.....x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            swing = 0.2f; dly = 0.2f; break;
        case 37: // Synth Killer
            setRow (sequencer, 0, "x..x..x...x..x..");
            setRow (sequencer, 3, "........x.......");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            cru = 0.3f; break;
        case 38: // Cyber Bounce
            setRow (sequencer, 0, "x..x..x..x..x..x");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 5, "x.x.x.x.x.x.x.x.");
            cyb = 0.35f; break;
        case 39: // Electric Surge
            setRow (sequencer, 0, "x.x...x.x.x...x.");
            setRow (sequencer, 2, ".x.x.x.x.x.x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            rvb = 0.4f; break;
        case 40: // Pulsing Core
            setRow (sequencer, 0, "x...x...x...x...");
            setRow (sequencer, 3, "....x.......x...");
            setRow (sequencer, 5, "..x...x...x...x.");
            dly = 0.3f; break;
        case 41: // Neon Frenzy
            setRow (sequencer, 0, "x.x.x...x.x.x...");
            setRow (sequencer, 2, "....x.x.....x.x.");
            setRow (sequencer, 4, "xxxxxxxxxxxxxxxx");
            gli = 0.25f; break;
        case 42: // Digital Breakdown
            setRow (sequencer, 0, "x..x..x..x..x..x");
            setRow (sequencer, 2, ".x.x.x.x.x.x.x.");
            setRow (sequencer, 5, "x.x.x.x.x.x.x.x.");
            mode = 5.0f; break;
        case 43: // Synth Prophet
            setRow (sequencer, 0, "x.......x.......");
            setRow (sequencer, 3, "........x.......");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            rvb = 0.35f; dly = 0.25f; break;
        case 44: // Glitch Funk
            setRow (sequencer, 0, "x...x...x...x...");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 15, "x.x.x.x.x.x.x.x.");
            gli = 0.4f; cru = 0.2f; break;
        case 45: // Neon Carnival
            setRow (sequencer, 0, "x..x..x..x..x..x");
            setRow (sequencer, 3, "....x.x.....x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            swing = 0.3f; break;
        case 46: // Synth Horizon
            setRow (sequencer, 0, "x.x.x.x.x.x.x.x.");
            setRow (sequencer, 2, "....x.......x...");
            setRow (sequencer, 5, "..x...x...x...x.");
            cyb = 0.2f; rvb = 0.3f; break;
        case 47: // Electric Monk
            setRow (sequencer, 0, "x...x.x.x...x.x.");
            setRow (sequencer, 2, ".x.x.x.x.x.x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            dly = 0.35f; break;
        case 48: // Retro Digital
            setRow (sequencer, 0, "x..x..x...x..x..");
            setRow (sequencer, 3, "....x.......x...");
            setRow (sequencer, 5, "x.x.x.x.x.x.x.x.");
            mode = 1.0f; break;
        case 49: // Future Shock
            setRow (sequencer, 0, "x.x...x.x.x...x.");
            setRow (sequencer, 2, "....x.x.....x.x.");
            setRow (sequencer, 4, "x.x.x.x.x.x.x.x.");
            gli = 0.35f; break;
        default: break;
    }

    setParam (pid::mode, mode);
    setParam (pid::swing, swing);
    setParam (pid::humanize, hum);
    setParam (pid::fxReverb, rvb);
    setParam (pid::fxGlitch, gli);
    setParam (pid::fxCrush, cru);
    setParam (pid::fxDelay, dly);
    setParam (pid::fxCyber, cyb);
}

//==============================================================================
juce::ValueTree MPC2077AudioProcessor::captureState()
{
    juce::ValueTree state ("MPC2077_STATE");
    state.setProperty ("version", 1, nullptr);
    state.setProperty ("selectedPad", selectedPad.load(), nullptr);
    state.setProperty ("program", currentProgram, nullptr);
    state.appendChild (apvts.copyState(), nullptr);
    state.appendChild (engine.toValueTree(), nullptr);
    state.appendChild (sequencer.toValueTree(), nullptr);
    return state;
}

void MPC2077AudioProcessor::restoreState (const juce::ValueTree& state)
{
    if (! state.hasType ("MPC2077_STATE")) return;

    selectedPad.store ((int) state.getProperty ("selectedPad", 0));
    currentProgram = (int) state.getProperty ("program", 0);

    auto apvtsChild = state.getChildWithName (apvts.state.getType());
    if (apvtsChild.isValid())
        apvts.replaceState (apvtsChild);

    engine.fromValueTree (state.getChildWithName ("MPC_ENGINE"));
    sequencer.fromValueTree (state.getChildWithName ("MPC_SEQ"));
}

void MPC2077AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = captureState().createXml())
        copyXmlToBinary (*xml, destData);
}

void MPC2077AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        restoreState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
bool MPC2077AudioProcessor::savePresetToFile (const juce::File& file)
{
    if (auto xml = captureState().createXml())
        return xml->writeTo (file);
    return false;
}

bool MPC2077AudioProcessor::loadPresetFromFile (const juce::File& file)
{
    if (auto xml = juce::XmlDocument::parse (file))
    {
        restoreState (juce::ValueTree::fromXml (*xml));
        return true;
    }
    return false;
}

void MPC2077AudioProcessor::initPatch()
{
    engine.loadDefaultKit();
    sequencer.clearAll();
    sequencer.setNumSteps (16);
    auto setParam = [this] (const char* id, float v)
    {
        if (auto* p = apvts.getParameter (id))
            p->setValueNotifyingHost (apvts.getParameterRange (id).convertTo0to1 (v));
    };
    setParam (pid::fxReverb, 0.2f); setParam (pid::fxGlitch, 0.0f);
    setParam (pid::fxCrush, 0.0f);  setParam (pid::fxDelay, 0.1f);
    setParam (pid::fxCyber, 0.0f);  setParam (pid::swing, 0.0f);
    setParam (pid::humanize, 0.0f);
}

//==============================================================================
juce::AudioProcessorEditor* MPC2077AudioProcessor::createEditor()
{
    return new MPC2077AudioProcessorEditor (*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MPC2077AudioProcessor();
}
