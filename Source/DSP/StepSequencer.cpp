#include "StepSequencer.h"

using namespace mpc;

StepSequencer::StepSequencer()
{
    for (auto& s : steps) s.store (false);
    for (auto& v : vels)  v.store (0.8f);
}

void StepSequencer::prepare (double sampleRate)
{
    sr = sampleRate;
}

void StepSequencer::reset()
{
    internalPhase = 0.0;
    currentStep.store (-1);
    running.store (false);
}

//==============================================================================
static inline int seqIndex (int pad, int step)
{
    pad  = juce::jlimit (0, kNumPads - 1, pad);
    step = juce::jlimit (0, kMaxSteps - 1, step);
    return pad * kMaxSteps + step;
}

bool  StepSequencer::getStep (int pad, int step) const { return steps[(size_t) seqIndex (pad, step)].load(); }
void  StepSequencer::setStep (int pad, int step, bool on) { steps[(size_t) seqIndex (pad, step)].store (on); }
void  StepSequencer::toggleStep (int pad, int step) { const int i = seqIndex (pad, step); steps[(size_t) i].store (! steps[(size_t) i].load()); }
float StepSequencer::getVel (int pad, int step) const { return vels[(size_t) seqIndex (pad, step)].load(); }
void  StepSequencer::setVel (int pad, int step, float v) { vels[(size_t) seqIndex (pad, step)].store (juce::jlimit (0.0f, 1.0f, v)); }

void StepSequencer::clearPad (int pad)
{
    for (int s = 0; s < kMaxSteps; ++s)
        steps[(size_t) seqIndex (pad, s)].store (false);
}

void StepSequencer::clearAll()
{
    for (auto& s : steps) s.store (false);
}

//==============================================================================
juce::ValueTree StepSequencer::toValueTree() const
{
    juce::ValueTree tree ("MPC_SEQ");
    tree.setProperty ("numSteps", numSteps.load(), nullptr);
    tree.setProperty ("swing", swing.load(), nullptr);
    tree.setProperty ("humanize", humanize.load(), nullptr);

    for (int pad = 0; pad < kNumPads; ++pad)
    {
        juce::ValueTree row ("ROW");
        row.setProperty ("pad", pad, nullptr);

        juce::String mask, vstr;
        for (int s = 0; s < kMaxSteps; ++s)
        {
            mask << (steps[(size_t) seqIndex (pad, s)].load() ? '1' : '0');
            vstr << juce::String (juce::roundToInt (vels[(size_t) seqIndex (pad, s)].load() * 127.0f)) << ' ';
        }
        row.setProperty ("mask", mask, nullptr);
        row.setProperty ("vel", vstr.trim(), nullptr);
        tree.appendChild (row, nullptr);
    }
    return tree;
}

void StepSequencer::fromValueTree (const juce::ValueTree& tree)
{
    if (! tree.hasType ("MPC_SEQ")) return;

    numSteps.store (juce::jlimit (1, kMaxSteps, (int) tree.getProperty ("numSteps", 16)));
    swing.store    ((float) tree.getProperty ("swing", 0.0));
    humanize.store ((float) tree.getProperty ("humanize", 0.0));

    clearAll();
    for (int c = 0; c < tree.getNumChildren(); ++c)
    {
        auto row = tree.getChild (c);
        if (! row.hasType ("ROW")) continue;
        const int pad = (int) row.getProperty ("pad", -1);
        if (pad < 0 || pad >= kNumPads) continue;

        const juce::String mask = row.getProperty ("mask", juce::String());
        for (int s = 0; s < juce::jmin (kMaxSteps, mask.length()); ++s)
            steps[(size_t) seqIndex (pad, s)].store (mask[s] == '1');

        const juce::String vstr = row.getProperty ("vel", juce::String());
        auto toks = juce::StringArray::fromTokens (vstr, " ", "");
        for (int s = 0; s < juce::jmin (kMaxSteps, toks.size()); ++s)
            vels[(size_t) seqIndex (pad, s)].store (juce::jlimit (0.0f, 1.0f, toks[s].getIntValue() / 127.0f));
    }
}
