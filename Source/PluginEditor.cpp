#include "PluginEditor.h"

using namespace mpc;
using namespace mpc::theme;

namespace { constexpr int kW = 980, kH = 620; }

MPC2077AudioProcessorEditor::MPC2077AudioProcessorEditor (MPC2077AudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      topPanel (p), mixer (p), padGrid (p), seqView (p),
      envSection (p, "ENVELOPE"), toneSection (p, "TONE"), fxSection (p, "FX")
{
    setLookAndFeel (&lnf);

    // ENVELOPE (selected pad)
    envSection.addPadKnob ("ATTACK",  { 0.0, 0.5, 0.0, 0.4 },
        [] (const PadSettings& pd) { return (double) pd.attack; },
        [] (PadSettings& pd, double v) { pd.attack = (float) v; });
    envSection.addPadKnob ("DECAY",   { 0.005, 2.0, 0.0, 0.4 },
        [] (const PadSettings& pd) { return (double) pd.decay; },
        [] (PadSettings& pd, double v) { pd.decay = (float) v; });
    envSection.addPadKnob ("SUSTAIN", { 0.0, 1.0 },
        [] (const PadSettings& pd) { return (double) pd.sustain; },
        [] (PadSettings& pd, double v) { pd.sustain = (float) v; });
    envSection.addPadKnob ("RELEASE", { 0.005, 2.0, 0.0, 0.4 },
        [] (const PadSettings& pd) { return (double) pd.release; },
        [] (PadSettings& pd, double v) { pd.release = (float) v; });

    // TONE (selected pad)
    toneSection.addPadKnob ("PITCH",  { -24.0, 24.0 },
        [] (const PadSettings& pd) { return (double) pd.pitchSemis; },
        [] (PadSettings& pd, double v) { pd.pitchSemis = (float) v; });
    toneSection.addPadKnob ("FILTER", { 0.0, 1.0 },
        [] (const PadSettings& pd) { return (double) pd.cutoff; },
        [] (PadSettings& pd, double v) { pd.cutoff = (float) v; });
    toneSection.addPadKnob ("RESO",   { 0.0, 1.0 },
        [] (const PadSettings& pd) { return (double) pd.reso; },
        [] (PadSettings& pd, double v) { pd.reso = (float) v; });
    toneSection.addPadKnob ("DRIVE",  { 0.0, 1.0 },
        [] (const PadSettings& pd) { return (double) pd.drive; },
        [] (PadSettings& pd, double v) { pd.drive = (float) v; });

    // FX (master, APVTS)
    fxSection.addParamKnob ("REVERB", pid::fxReverb);
    fxSection.addParamKnob ("DELAY",  pid::fxDelay);
    fxSection.addParamKnob ("CRUSH",  pid::fxCrush);
    fxSection.addParamKnob ("GLITCH", pid::fxGlitch);
    fxSection.addParamKnob ("CYBER",  pid::fxCyber);

    addAndMakeVisible (topPanel);
    addAndMakeVisible (mixer);
    addAndMakeVisible (padGrid);
    addChildComponent (seqView);
    addAndMakeVisible (envSection);
    addAndMakeVisible (toneSection);
    addAndMakeVisible (fxSection);

    padGrid.onSelectionChanged = [this] { envSection.refresh(); toneSection.refresh(); mixer.repaint(); };
    mixer.onSelect             = [this] { envSection.refresh(); toneSection.refresh(); padGrid.repaint(); };
    topPanel.onPatchChanged    = [this] { refreshForNewPatch(); };
    topPanel.onSeqToggle       = [this] (bool on) { setSeqView (on); };

    setResizable (false, false);
    setSize (kW, kH);
    startTimerHz (24);
    refreshForNewPatch();
}

MPC2077AudioProcessorEditor::~MPC2077AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void MPC2077AudioProcessorEditor::setSeqView (bool on)
{
    seqView.setVisible (on);
    padGrid.setVisible (! on);
    repaint();
}

void MPC2077AudioProcessorEditor::refreshForNewPatch()
{
    envSection.refresh();
    toneSection.refresh();
    mixer.refresh();
    padGrid.repaint();
    topPanel.repaint();
    lastSelectedPad = proc.selectedPad.load();
    lastProgram = proc.getCurrentProgram();
}

//==============================================================================
void MPC2077AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bg);
    circuitGrid (g, getLocalBounds().toFloat(), 34.0f);

    // section title above the pad / sequencer area
    g.setFont (theme::hud (11.0f, true));
    glowText (g, seqView.isVisible() ? "STEP SEQUENCER" : "DRUM PADS",
              juce::Rectangle<int> (12, 176, 560, 18), cyan, juce::Justification::centredLeft, 0.55f);
}

void MPC2077AudioProcessorEditor::resized()
{
    topPanel.setBounds (0, 0, kW, 86);
    mixer.setBounds (12, 92, kW - 24, 78);

    const juce::Rectangle<int> leftArea (12, 196, 560, 410);
    padGrid.setBounds (leftArea);
    seqView.setBounds (leftArea);

    const int rx = 584, rw = kW - rx - 12;   // right column
    envSection.setBounds  (rx, 178, rw, 132);
    toneSection.setBounds (rx, 318, rw, 132);
    fxSection.setBounds   (rx, 450, rw, 156);
}

//==============================================================================
void MPC2077AudioProcessorEditor::timerCallback()
{
    const int sel = proc.selectedPad.load();
    if (sel != lastSelectedPad)
    {
        lastSelectedPad = sel;
        envSection.refresh(); toneSection.refresh();
        mixer.repaint(); padGrid.repaint();
    }

    const int prog = proc.getCurrentProgram();
    if (prog != lastProgram) { lastProgram = prog; refreshForNewPatch(); }
}
