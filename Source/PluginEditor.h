#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/MPC2077LookAndFeel.h"
#include "UI/Components/TopPanel.h"
#include "UI/Components/MixerStrip.h"
#include "UI/Components/PadGrid.h"
#include "UI/Components/SequencerView.h"
#include "UI/Components/ControlSection.h"

class MPC2077AudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit MPC2077AudioProcessorEditor (MPC2077AudioProcessor&);
    ~MPC2077AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void refreshForNewPatch();
    void setSeqView (bool on);

    MPC2077AudioProcessor& proc;
    mpc::MPC2077LookAndFeel lnf;

    mpc::TopPanel       topPanel;
    mpc::MixerStrip     mixer;
    mpc::PadGrid        padGrid;
    mpc::SequencerView  seqView;
    mpc::ControlSection envSection, toneSection, fxSection;

    int lastSelectedPad = -1;
    int lastProgram = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPC2077AudioProcessorEditor)
};
