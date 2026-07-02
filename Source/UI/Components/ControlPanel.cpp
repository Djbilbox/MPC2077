#include "ControlPanel.h"
#include "../../PluginProcessor.h"

using namespace mpc;
using namespace mpc::theme;

// knob index -> value range
static juce::NormalisableRange<double> knobRange (int k)
{
    switch (k)
    {
        case 5: return { 0.02, 2.0, 0.0, 0.4 };   // DEPTH  = decay seconds (skewed)
        case 6: return { -12.0, 12.0 };           // DETUNE = semitones
        default: return { 0.0, 1.0 };             // 0..1 knobs
    }
}

ControlPanel::ControlPanel (MPC2077AudioProcessor& p) : proc (p)
{
    for (int i = 0; i < kNum; ++i)
    {
        auto& s = knobs[i];
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        s.setNormalisableRange (knobRange (i));
        s.setDoubleClickReturnValue (true, (i == 2 ? 1.0 : (i == 6 ? 0.0 : 0.0)));
        s.onValueChange = [this, i] { writeToPad (i, knobs[i].getValue()); };
        addAndMakeVisible (s);
    }
    refresh();
}

void ControlPanel::writeToPad (int k, double v)
{
    auto& pad = proc.getEngine().pad (proc.selectedPad.load());
    switch (k)
    {
        case 0: pad.glide      = (float) v; break;
        case 1: pad.reverbSend = (float) v; break;
        case 2: pad.cutoff     = (float) v; break;
        case 3: pad.drive      = (float) v; break;
        case 4: pad.reso       = (float) v; break;
        case 5: pad.decay      = (float) v; break;
        case 6: pad.pitchSemis = (float) v; break;
        case 7: pad.delaySend  = (float) v; break;
        default: break;
    }
}

void ControlPanel::refresh()
{
    const auto& pad = proc.getEngine().pad (proc.selectedPad.load());
    const double vals[kNum] = { pad.glide, pad.reverbSend, pad.cutoff, pad.drive,
                                pad.reso, pad.decay, pad.pitchSemis, pad.delaySend };
    for (int i = 0; i < kNum; ++i)
        knobs[i].setValue (vals[i], juce::dontSendNotification);
    repaint();
}

//==============================================================================
void ControlPanel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (panel);
    g.fillRoundedRectangle (r, 8.0f);
    circuitGrid (g, r.reduced (4.0f), 22.0f);
    g.setColour (cyan.withAlpha (0.35f));
    g.drawRoundedRectangle (r.reduced (1.0f), 8.0f, 1.2f);

    glowText (g, "CONTROLS", getLocalBounds().removeFromTop (24).reduced (4, 2), cyan,
              juce::Justification::centred, 0.6f);

    // knob labels + selected pad name
    g.setFont (theme::hud (9.5f, true));
    for (int i = 0; i < kNum; ++i)
    {
        auto kb = knobs[i].getBounds();
        g.setColour (textHi.withAlpha (0.9f));
        g.drawText (labels[i], kb.getX() - 4, kb.getBottom() - 2, kb.getWidth() + 8, 14,
                    juce::Justification::centred, false);
    }

    g.setColour (pink.withAlpha (0.9f));
    g.setFont (theme::hud (9.0f, true));
    g.drawText ("PAD " + juce::String (proc.selectedPad.load() + 1) + ": " + proc.getEngine().pad (proc.selectedPad.load()).name,
                getLocalBounds().removeFromBottom (18).reduced (4, 2), juce::Justification::centred, false);
}

void ControlPanel::resized()
{
    auto area = getLocalBounds().reduced (10);
    area.removeFromTop (22);        // CONTROLS header
    area.removeFromBottom (20);     // pad name footer

    const int cols = 2, rows = 4;
    const int cw = area.getWidth() / cols;
    const int ch = area.getHeight() / rows;
    for (int i = 0; i < kNum; ++i)
    {
        const int col = i % cols, row = i / cols;
        juce::Rectangle<int> cell (area.getX() + col * cw, area.getY() + row * ch, cw, ch);
        knobs[i].setBounds (cell.reduced (6).withTrimmedBottom (12));
    }
}
