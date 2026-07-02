#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

namespace mpc
{
/** Non-interactive Tokyo-megacity backdrop with a glitchy "MPC2077" watermark. */
class CityBackdrop : public juce::Component
{
public:
    CityBackdrop();
    void paint (juce::Graphics&) override;
    void resized() override;
    void setPhase (float p) { phase = p; }

private:
    juce::Image cache;
    float phase = 0.0f;
    juce::Random rng { 0x2077 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CityBackdrop)
};
}
