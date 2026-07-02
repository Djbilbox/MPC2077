#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** Right panel: FX CHAIN — 5 glowing vertical sliders bound to the master FX params. */
class FxRack : public juce::Component
{
public:
    explicit FxRack (MPC2077AudioProcessor& p);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;
    static constexpr int kNum = 5;

    MPC2077AudioProcessor& proc;
    juce::Slider sliders[kNum];
    std::unique_ptr<Attach> attach[kNum];
    const juce::StringArray labels { "REVERB", "GLITCH", "CRUSH", "DELAY", "CYBER FX" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxRack)
};
}
