#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** Top bar: LCD preset display + nav, MPC2077 logo/wordmark, master VOL, and
    SAVE / LOAD / INIT / SEQ buttons. */
class TopPanel : public juce::Component,
                 private juce::Timer
{
public:
    explicit TopPanel (MPC2077AudioProcessor& p);
    ~TopPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    std::function<void()> onPatchChanged;   // preset navigated -> refresh editor
    std::function<void(bool)> onSeqToggle;  // sequencer view on/off
    bool isSeqActive() const { return seqActive; }

private:
    void timerCallback() override;
    void step (int dir);
    void showPresetMenu();
    juce::Rectangle<int> displayBounds() const { return { 12, 8, 300, 40 }; }

    MPC2077AudioProcessor& proc;
    juce::Image logo;
    juce::Slider volKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volAttach;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" },
                     saveBtn { "SAVE" }, loadBtn { "LOAD" }, initBtn { "INIT" }, seqBtn { "SEQ" };
    std::unique_ptr<juce::FileChooser> chooser;

    bool  seqActive = false;
    int   lastProgram = -1;
    float phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopPanel)
};
}
