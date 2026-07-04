#pragma once

#include <JuceHeader.h>
#include "../Theme.h"
#include "../../DSP/DrumEngine.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** A titled horizontal bank of neon knobs. Knobs bind either to an APVTS
    parameter (master FX) or to a field of the currently selected pad. */
class ControlSection : public juce::Component
{
public:
    ControlSection (MPC2077AudioProcessor& p, const juce::String& title);

    void addParamKnob (const juce::String& label, const char* paramId);
    void addPadKnob   (const juce::String& label, juce::NormalisableRange<double> range,
                       std::function<double(const PadSettings&)> getter,
                       std::function<void(PadSettings&, double)> setter);

    void refresh();   // pull pad-bound knobs from the selected pad
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        std::unique_ptr<juce::Slider> slider;
        juce::String label;
        bool isPad = false;
        std::function<double(const PadSettings&)> getter;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    MPC2077AudioProcessor& proc;
    juce::String title;
    juce::OwnedArray<Knob> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlSection)
};
}
