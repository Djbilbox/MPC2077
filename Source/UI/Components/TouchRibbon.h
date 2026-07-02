#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** Footer: PITCH wheel (left), MOD wheel (right), playable touch ribbon (center). */
class TouchRibbon : public juce::Component
{
public:
    explicit TouchRibbon (MPC2077AudioProcessor& p);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp   (const juce::MouseEvent&) override;
    void setPhase (float ph) { phase = ph; }

private:
    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;

    void playAt (juce::Point<int> pos);
    juce::Rectangle<int> ribbonBounds() const;

    MPC2077AudioProcessor& proc;
    juce::Slider pitchSlider, modSlider;
    std::unique_ptr<Attach> pitchAttach, modAttach;
    float phase = 0.0f;
    float touchX = -1.0f;
    int   lastSemis = -1000;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TouchRibbon)
};
}
