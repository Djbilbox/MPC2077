#include "PresetBank.h"
#include "../../PluginProcessor.h"

using namespace mpc;
using namespace mpc::theme;

namespace { constexpr int kHeaderH = 30; }

PresetBank::PresetBank (MPC2077AudioProcessor& p) : proc (p) {}

juce::Rectangle<int> PresetBank::listArea() const
{
    return getLocalBounds().withTrimmedTop (kHeaderH).reduced (6, 2);
}

void PresetBank::paint (juce::Graphics& g)
{
    // header
    glowText (g, "// PRESET BANK //", getLocalBounds().removeFromTop (kHeaderH), cyan,
              juce::Justification::centred, 0.6f);

    auto area = listArea();
    const int cur = proc.getCurrentProgram();
    const int rowH = juce::jmax (18, area.getHeight() / FactoryPresetCount);

    g.setFont (theme::hud (13.0f, false));
    for (int i = 0; i < FactoryPresetCount; ++i)
    {
        juce::Rectangle<int> row (area.getX(), area.getY() + i * rowH, area.getWidth(), rowH);
        const bool sel = (i == cur);

        if (sel)
        {
            g.setColour (pink.withAlpha (0.16f));
            g.fillRoundedRectangle (row.toFloat().reduced (1.0f), 4.0f);
            g.setColour (pink.withAlpha (0.8f));
            g.drawRoundedRectangle (row.toFloat().reduced (1.0f), 4.0f, 1.2f);
        }

        // number
        g.setColour ((sel ? pink : cyan).withAlpha (0.7f));
        g.setFont (theme::hud (10.0f, true));
        g.drawText (juce::String (i + 1).paddedLeft ('0', 2),
                    row.withWidth (44).reduced (10, 0), juce::Justification::centredLeft, false);

        // name
        g.setFont (theme::hud (13.0f, sel));
        if (sel) glowText (g, MPC2077AudioProcessor::factoryPresetName (i), row.withTrimmedLeft (48),
                           juce::Colours::white, juce::Justification::centredLeft, 0.7f);
        else { g.setColour (cyan.withAlpha (0.85f));
               g.drawText (MPC2077AudioProcessor::factoryPresetName (i), row.withTrimmedLeft (48),
                           juce::Justification::centredLeft, false); }
    }
}

void PresetBank::mouseDown (const juce::MouseEvent& e)
{
    auto area = listArea();
    if (! area.contains (e.getPosition())) return;
    const int rowH = juce::jmax (18, area.getHeight() / FactoryPresetCount);
    const int row = (e.getPosition().y - area.getY()) / rowH;
    if (row >= 0 && row < FactoryPresetCount)
    {
        proc.setCurrentProgram (row);
        if (onPresetChosen) onPresetChosen();
        repaint();
    }
}
