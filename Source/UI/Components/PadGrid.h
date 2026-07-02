#pragma once

#include <JuceHeader.h>
#include "../Theme.h"
#include "../../DSP/MpcConstants.h"

class MPC2077AudioProcessor;

namespace mpc
{
/** 4x4 MPC pad grid: trigger, select, drag-&-drop sample load, double-click browse. */
class PadGrid : public juce::Component,
                public juce::FileDragAndDropTarget,
                private juce::Timer
{
public:
    explicit PadGrid (MPC2077AudioProcessor& p);
    ~PadGrid() override;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void fileDragEnter (const juce::StringArray&, int, int) override { repaint(); }
    void fileDragExit  (const juce::StringArray&) override { dragPad = -1; repaint(); }
    void fileDragMove  (const juce::StringArray&, int x, int y) override;

    std::function<void()> onSelectionChanged;

private:
    void timerCallback() override;
    int  padAt (juce::Point<int> pos) const;
    juce::Rectangle<float> padBounds (int index) const;
    void loadFileIntoPad (int pad, const juce::File&);

    MPC2077AudioProcessor& proc;
    std::array<float, kNumPads> flash {};
    int dragPad = -1;
    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PadGrid)
};
}
