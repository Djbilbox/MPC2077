#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/MPC2077LookAndFeel.h"
#include "UI/Components/ModeBar.h"
#include "UI/Components/ControlPanel.h"
#include "UI/Components/CityBackdrop.h"
#include "UI/Components/PresetBank.h"
#include "UI/Components/PadGrid.h"
#include "UI/Components/FxRack.h"
#include "UI/Components/TouchRibbon.h"

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

    MPC2077AudioProcessor& proc;
    mpc::MPC2077LookAndFeel lnf;

    mpc::ModeBar      modeBar;
    mpc::ControlPanel controlPanel;
    mpc::CityBackdrop city;
    mpc::PresetBank   presetBank;
    mpc::PadGrid      padGrid;
    mpc::FxRack       fxRack;
    mpc::TouchRibbon  ribbon;

    juce::TextButton saveBtn { "SAVE" }, loadBtn { "LOAD" }, initBtn { "INIT" };
    std::unique_ptr<juce::FileChooser> chooser;

    float phase = 0.0f;
    int   lastSelectedPad = -1;
    int   lastProgram = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPC2077AudioProcessorEditor)
};
