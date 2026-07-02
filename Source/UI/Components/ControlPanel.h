#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** Left panel: 8 holographic knobs that edit the CURRENTLY SELECTED pad. */
class ControlPanel : public juce::Component
{
public:
    explicit ControlPanel (MPC2077AudioProcessor& p);

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Pull the selected pad's values into the knobs (call when selection changes). */
    void refresh();

private:
    static constexpr int kNum = 8;
    MPC2077AudioProcessor& proc;
    juce::Slider knobs[kNum];
    const juce::StringArray labels { "GLIDE", "SPACE", "FILTER", "CYBER", "RESO", "DEPTH", "DETUNE", "FX MIX" };

    void writeToPad (int knob, double value);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
}
