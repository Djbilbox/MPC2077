#pragma once

#include <JuceHeader.h>
#include "../Theme.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** Top bar: chrome MPC2077 logo, 6 mode pills (CHOP/SLICE/BEAT/SEQ/GLITCH/CYBER), status. */
class ModeBar : public juce::Component,
                private juce::Timer
{
public:
    explicit ModeBar (MPC2077AudioProcessor& p);
    ~ModeBar() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void setPhase (float p) { phase = p; }

private:
    void timerCallback() override;
    void setMode (int index);

    MPC2077AudioProcessor& proc;
    juce::TextButton pills[6];
    const juce::StringArray names { "CHOP", "SLICE", "BEAT", "SEQ", "GLITCH", "CYBER" };
    juce::Image logo;
    float phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModeBar)
};
}
