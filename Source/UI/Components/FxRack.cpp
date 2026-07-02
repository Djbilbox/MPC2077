#include "FxRack.h"
#include "../../PluginProcessor.h"

using namespace mpc;
using namespace mpc::theme;

FxRack::FxRack (MPC2077AudioProcessor& p) : proc (p)
{
    const char* ids[kNum] = { pid::fxReverb, pid::fxGlitch, pid::fxCrush, pid::fxDelay, pid::fxCyber };
    for (int i = 0; i < kNum; ++i)
    {
        auto& s = sliders[i];
        s.setSliderStyle (juce::Slider::LinearVertical);
        s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        s.onValueChange = [this] { repaint(); };
        addAndMakeVisible (s);
        attach[i] = std::make_unique<Attach> (proc.getAPVTS(), ids[i], s);
    }
}

void FxRack::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (panel);
    g.fillRoundedRectangle (r, 8.0f);
    circuitGrid (g, r.reduced (4.0f), 22.0f);
    g.setColour (cyan.withAlpha (0.35f));
    g.drawRoundedRectangle (r.reduced (1.0f), 8.0f, 1.2f);

    glowText (g, "FX CHAIN", getLocalBounds().removeFromTop (24).reduced (4, 2), cyan,
              juce::Justification::centred, 0.6f);

    for (int i = 0; i < kNum; ++i)
    {
        auto sb = sliders[i].getBounds();
        // percentage above
        g.setColour (cyan);
        g.setFont (theme::hud (10.0f, true));
        g.drawText (juce::String (juce::roundToInt (sliders[i].getValue() * 100.0)) + "%",
                    sb.getX() - 6, sb.getY() - 16, sb.getWidth() + 12, 14, juce::Justification::centred, false);
        // label below
        g.setColour (textHi.withAlpha (0.9f));
        g.setFont (theme::hud (8.5f, true));
        g.drawText (labels[i], sb.getX() - 8, sb.getBottom() + 2, sb.getWidth() + 16, 20,
                    juce::Justification::centredTop, false);
    }
}

void FxRack::resized()
{
    auto area = getLocalBounds().reduced (8);
    area.removeFromTop (26);        // FX CHAIN header
    area.removeFromTop (16);        // value row
    area.removeFromBottom (22);     // labels

    const int cw = area.getWidth() / kNum;
    for (int i = 0; i < kNum; ++i)
        sliders[i].setBounds (area.getX() + i * cw, area.getY(), cw, area.getHeight());
}
