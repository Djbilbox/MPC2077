#pragma once

#include <JuceHeader.h>
#include "../Theme.h"
#include "../../DSP/MpcConstants.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** Horizontal mixer: one small level knob per pad (APESHYT-style MIXER row). */
class MixerStrip : public juce::Component
{
public:
    explicit MixerStrip (MPC2077AudioProcessor& p);

    void paint (juce::Graphics&) override;
    void resized() override;
    void refresh();

    std::function<void()> onSelect;   // fired when a mixer knob selects a pad

    static juce::String shortName (int pad);

private:
    MPC2077AudioProcessor& proc;
    juce::Slider knobs[kNumPads];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerStrip)
};
}
