#include "MixerStrip.h"
#include "../../PluginProcessor.h"

using namespace mpc;
using namespace mpc::theme;

juce::String MixerStrip::shortName (int pad)
{
    static const char* s[kNumPads] = {
        "KK", "808", "SN", "CP", "CH", "OH", "LT", "MT",
        "HT", "RM", "CR", "RD", "CV", "CB", "CG", "SP"
    };
    return juce::String (s[juce::jlimit (0, kNumPads - 1, pad)]);
}

MixerStrip::MixerStrip (MPC2077AudioProcessor& p) : proc (p)
{
    for (int i = 0; i < kNumPads; ++i)
    {
        auto& s = knobs[i];
        s.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        s.setNormalisableRange ({ 0.0, 1.5 });
        s.setDoubleClickReturnValue (true, 0.85);
        s.onValueChange = [this, i] { proc.getEngine().pad (i).gain = (float) knobs[i].getValue(); };
        s.onDragStart   = [this, i] { proc.selectedPad.store (i); if (onSelect) onSelect(); };
        addAndMakeVisible (s);
    }
    refresh();
}

void MixerStrip::refresh()
{
    for (int i = 0; i < kNumPads; ++i)
        knobs[i].setValue (proc.getEngine().pad (i).gain, juce::dontSendNotification);
    repaint();
}

void MixerStrip::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (panel);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (cyan.withAlpha (0.30f));
    g.drawRoundedRectangle (r.reduced (0.8f), 6.0f, 1.0f);

    glowText (g, "MIXER", getLocalBounds().removeFromTop (16).reduced (8, 1),
              cyan, juce::Justification::centredLeft, 0.55f);

    const int sel = proc.selectedPad.load();
    g.setFont (theme::hud (8.0f, true));
    for (int i = 0; i < kNumPads; ++i)
    {
        auto kb = knobs[i].getBounds();
        g.setColour ((i == sel ? pink : textHi).withAlpha (0.9f));
        g.drawText (shortName (i), kb.getX() - 4, kb.getBottom() - 1, kb.getWidth() + 8, 12,
                    juce::Justification::centred, false);
    }
}

void MixerStrip::resized()
{
    auto area = getLocalBounds().reduced (6, 2);
    area.removeFromTop (15);
    const int cw = area.getWidth() / kNumPads;
    for (int i = 0; i < kNumPads; ++i)
        knobs[i].setBounds (juce::Rectangle<int> (area.getX() + i * cw, area.getY(), cw, area.getHeight())
                                .reduced (3).withTrimmedBottom (11));
}
