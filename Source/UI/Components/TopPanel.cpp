#include "TopPanel.h"
#include "../../PluginProcessor.h"

#if MPC_HAS_BINARY_DATA
 #include "BinaryData.h"
#endif

using namespace mpc;
using namespace mpc::theme;

TopPanel::TopPanel (MPC2077AudioProcessor& p) : proc (p)
{
   #if MPC_HAS_BINARY_DATA
    logo = juce::ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize);
   #endif

    volKnob.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    volKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible (volKnob);
    volAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.getAPVTS(), pid::master, volKnob);

    prevBtn.onClick = [this] { step (-1); };
    nextBtn.onClick = [this] { step (+1); };
    initBtn.onClick = [this] { proc.initPatch(); if (onPatchChanged) onPatchChanged(); };

    saveBtn.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Save MPC2077 preset",
                    juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.mpcpreset");
        chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            { auto f = fc.getResult(); if (f != juce::File{}) proc.savePresetToFile (f.withFileExtension (".mpcpreset")); });
    };
    loadBtn.onClick = [this]
    {
        chooser = std::make_unique<juce::FileChooser> ("Load MPC2077 preset",
                    juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.mpcpreset");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this] (const juce::FileChooser& fc)
            { auto f = fc.getResult(); if (f.existsAsFile() && proc.loadPresetFromFile (f) && onPatchChanged) onPatchChanged(); });
    };
    seqBtn.onClick = [this]
    {
        seqActive = ! seqActive;
        seqBtn.setToggleState (seqActive, juce::dontSendNotification);
        if (onSeqToggle) onSeqToggle (seqActive);
    };

    for (auto* b : { &prevBtn, &nextBtn, &saveBtn, &loadBtn, &initBtn, &seqBtn })
        addAndMakeVisible (b);

    startTimerHz (12);
}
TopPanel::~TopPanel() { stopTimer(); }

void TopPanel::step (int dir)
{
    const int n = proc.getNumPrograms();
    int prog = (proc.getCurrentProgram() + dir + n) % n;
    proc.setCurrentProgram (prog);
    if (onPatchChanged) onPatchChanged();
    repaint();
}

void TopPanel::showPresetMenu()
{
    juce::PopupMenu m;
    const int cur = proc.getCurrentProgram();
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        m.addItem (i + 1, MPC2077AudioProcessor::factoryPresetName (i), true, i == cur);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
        [this] (int r) { if (r > 0) { proc.setCurrentProgram (r - 1); if (onPatchChanged) onPatchChanged(); repaint(); } });
}

void TopPanel::mouseDown (const juce::MouseEvent& e)
{
    if (displayBounds().contains (e.getPosition()))
        showPresetMenu();
}

void TopPanel::timerCallback()
{
    phase += 0.05f;
    const int prog = proc.getCurrentProgram();
    if (prog != lastProgram) { lastProgram = prog; }
    repaint();
}

void TopPanel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xFF0C0C1A), 0, 0,
                                             juce::Colour (0xFF05000F), 0, r.getHeight(), false));
    g.fillRect (r);
    g.setColour (cyan.withAlpha (0.5f));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, r.getWidth());

    // --- LCD preset display ---
    auto d = displayBounds();
    g.setColour (juce::Colour (0xFF04120F));
    g.fillRoundedRectangle (d.toFloat(), 4.0f);
    g.setColour (cyan.withAlpha (0.6f));
    g.drawRoundedRectangle (d.toFloat().reduced (0.6f), 4.0f, 1.0f);
    // scanlines
    g.setColour (cyan.withAlpha (0.05f));
    for (int y = d.getY() + 2; y < d.getBottom(); y += 3) g.drawHorizontalLine (y, (float) d.getX(), (float) d.getRight());

    g.setColour (cyan.withAlpha (0.75f));
    g.setFont (theme::hud (9.0f, true));
    g.drawText ("PRESET " + juce::String (proc.getCurrentProgram() + 1).paddedLeft ('0', 2)
                + " / " + juce::String (proc.getNumPrograms()),
                d.reduced (8, 4).removeFromTop (14), juce::Justification::topLeft, false);
    g.setFont (theme::hud (15.0f, true));
    glowText (g, MPC2077AudioProcessor::factoryPresetName (proc.getCurrentProgram()),
              d.reduced (8, 3).withTrimmedTop (13), juce::Colours::white, juce::Justification::centredLeft, 0.5f);

    // --- circular glow-ring badge + wordmark (centred) ---
    const int cx = getWidth() / 2 - 40;
    const float bcx = (float) cx, bcy = 40.0f, brad = 34.0f;
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (cyan.withAlpha (0.05f * (float) i));
        g.drawEllipse (bcx - brad - i * 2.0f, bcy - brad - i * 2.0f, (brad + i * 2.0f) * 2.0f, (brad + i * 2.0f) * 2.0f, 2.0f);
    }
    g.setColour (juce::Colour (0xFF0A0018));
    g.fillEllipse (bcx - brad, bcy - brad, brad * 2.0f, brad * 2.0f);
    g.setColour (cyan.withAlpha (0.7f));
    g.drawEllipse (bcx - brad, bcy - brad, brad * 2.0f, brad * 2.0f, 1.4f);
    if (logo.isValid())
        g.drawImageWithin (logo, (int) (bcx - brad), (int) (bcy - brad), (int) (brad * 2.0f), (int) (brad * 2.0f),
                           juce::RectanglePlacement::centred, false);

    auto wm = juce::Rectangle<int> (cx + 46, 14, 260, 40);
    const float gj = (std::sin (phase * 6.0f) > 0.9f) ? 3.0f : 0.0f;
    g.setFont (theme::hud (30.0f, true));
    g.setColour (pink.withAlpha (0.55f)); g.drawText ("MPC2077", wm.translated ((int) gj, 0), juce::Justification::centredLeft, false);
    g.setColour (cyan.withAlpha (0.55f)); g.drawText ("MPC2077", wm.translated ((int) -gj, 0), juce::Justification::centredLeft, false);
    g.setColour (textHi);                 g.drawText ("MPC2077", wm, juce::Justification::centredLeft, false);
    g.setFont (theme::hud (9.0f, true));
    g.setColour (pink);
    g.drawText ("by Djbilbox BEATS", cx + 48, 52, 240, 12, juce::Justification::centredLeft, false);

    // VOL label
    g.setColour (cyan);
    g.setFont (theme::hud (9.0f, true));
    g.drawText ("VOL", volKnob.getX() - 6, volKnob.getBottom() - 2, volKnob.getWidth() + 12, 12,
                juce::Justification::centred, false);
}

void TopPanel::resized()
{
    // buttons row under the display
    const int by = 52, bh = 22;
    prevBtn.setBounds (12,  by, 26, bh);
    nextBtn.setBounds (40,  by, 26, bh);
    saveBtn.setBounds (74,  by, 54, bh);
    loadBtn.setBounds (130, by, 54, bh);
    initBtn.setBounds (186, by, 50, bh);
    seqBtn.setBounds  (240, by, 54, bh);

    volKnob.setBounds (getWidth() - 78, 6, 64, 60);
}
