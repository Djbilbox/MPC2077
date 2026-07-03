#pragma once

#include <JuceHeader.h>
#include "../Theme.h"
#include "../../DSP/MpcConstants.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** Step-sequencer editor: 16 pads x N steps matrix, playhead, swing/humanize, 16/32 toggle. */
class SequencerView : public juce::Component,
                      private juce::Timer
{
public:
    explicit SequencerView (MPC2077AudioProcessor& p);
    ~SequencerView() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;

private:
    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;

    void timerCallback() override;
    juce::Rectangle<int> gridArea() const;
    bool cellAt (juce::Point<int> pos, int& pad, int& step) const;

    MPC2077AudioProcessor& proc;
    juce::TextButton stepsBtn { "16" }, clearBtn { "CLR" };
    juce::Slider swingSlider, humSlider;
    std::unique_ptr<Attach> swingAttach, humAttach;

    bool paintValue = true;   // value being painted during a drag
    int  lastStep = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SequencerView)
};
}
