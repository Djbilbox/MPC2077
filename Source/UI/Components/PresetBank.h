#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** Center preset list (translucent HUD over the city) + "// PRESET BANK //" header. */
class PresetBank : public juce::Component
{
public:
    explicit PresetBank (MPC2077AudioProcessor& p);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

    std::function<void()> onPresetChosen;

private:
    juce::Rectangle<int> listArea() const;

    MPC2077AudioProcessor& proc;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBank)
};
}
