#include "PluginEditor.h"

using namespace mpc;
using namespace mpc::theme;

namespace { constexpr int kW = 1000, kH = 770; }

MPC2077AudioProcessorEditor::MPC2077AudioProcessorEditor (MPC2077AudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      modeBar (p), controlPanel (p), presetBank (p), padGrid (p), fxRack (p), ribbon (p)
{
    setLookAndFeel (&lnf);

    addAndMakeVisible (city);
    addAndMakeVisible (modeBar);
    addAndMakeVisible (controlPanel);
    addAndMakeVisible (presetBank);
    addAndMakeVisible (padGrid);
    addAndMakeVisible (fxRack);
    addAndMakeVisible (ribbon);

    for (auto* b : { &saveBtn, &loadBtn, &initBtn }) addAndMakeVisible (b);

    padGrid.onSelectionChanged   = [this] { controlPanel.refresh(); };
    presetBank.onPresetChosen    = [this] { refreshForNewPatch(); };

    saveBtn.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Save MPC2077 preset",
                    juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.mpcpreset");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f != juce::File{}) proc.savePresetToFile (f.withFileExtension (".mpcpreset"));
            });
    };
    loadBtn.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Load MPC2077 preset",
                    juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.mpcpreset");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            {
                auto f = fc.getResult();
                if (f.existsAsFile() && proc.loadPresetFromFile (f)) refreshForNewPatch();
            });
    };
    initBtn.onClick = [this] { proc.initPatch(); refreshForNewPatch(); };

    setResizable (false, false);
    setSize (kW, kH);
    startTimerHz (24);
}

MPC2077AudioProcessorEditor::~MPC2077AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void MPC2077AudioProcessorEditor::refreshForNewPatch()
{
    controlPanel.refresh();
    presetBank.repaint();
    padGrid.repaint();
    lastSelectedPad = proc.selectedPad.load();
    lastProgram = proc.getCurrentProgram();
}

//==============================================================================
void MPC2077AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);

    // center panel frame + bank bar
    auto center = juce::Rectangle<int> (202, 80, 602, 602);
    g.setColour (panel.withAlpha (0.35f));
    g.fillRect (center);
    g.setColour (cyan.withAlpha (0.35f));
    g.drawRect (center, 1);

    // bank info bar
    auto bar = juce::Rectangle<int> (206, 388, 594, 28);
    g.setColour (panelLo.withAlpha (0.85f));
    g.fillRect (bar);
    g.setColour (cyan.withAlpha (0.6f));
    g.setFont (theme::hud (9.5f, true));
    g.drawText ("BANK A  -  10 PRESETS", bar.withTrimmedLeft (10), juce::Justification::centredLeft, false);
}

void MPC2077AudioProcessorEditor::resized()
{
    modeBar.setBounds (0, 0, kW, 74);

    controlPanel.setBounds (10, 80, 184, 602);
    fxRack.setBounds (806, 80, 184, 602);

    city.setBounds (202, 80, 602, 602);
    presetBank.setBounds (206, 84, 594, 300);

    // bank bar buttons (right side of the info bar at y=388)
    auto bar = juce::Rectangle<int> (206, 388, 594, 28);
    auto btns = bar.removeFromRight (200).reduced (2);
    const int bw = btns.getWidth() / 3;
    saveBtn.setBounds (btns.removeFromLeft (bw).reduced (2));
    loadBtn.setBounds (btns.removeFromLeft (bw).reduced (2));
    initBtn.setBounds (btns.reduced (2));

    padGrid.setBounds (212, 420, 582, 256);

    ribbon.setBounds (0, 686, kW, 84);
}

//==============================================================================
void MPC2077AudioProcessorEditor::timerCallback()
{
    phase += 0.05f;
    modeBar.setPhase (phase);
    city.setPhase (phase);
    ribbon.setPhase (phase);

    const int sel = proc.selectedPad.load();
    if (sel != lastSelectedPad) { lastSelectedPad = sel; controlPanel.refresh(); }

    const int prog = proc.getCurrentProgram();
    if (prog != lastProgram) { lastProgram = prog; refreshForNewPatch(); }

    modeBar.repaint();
    city.repaint();
    ribbon.repaint();
}
