#include "ModeBar.h"
#include "../../PluginProcessor.h"

#if MPC_HAS_BINARY_DATA
 #include "BinaryData.h"
#endif

using namespace mpc;
using namespace mpc::theme;

ModeBar::ModeBar (MPC2077AudioProcessor& p) : proc (p)
{
   #if MPC_HAS_BINARY_DATA
    logo = juce::ImageCache::getFromMemory (BinaryData::logo_png, BinaryData::logo_pngSize);
   #endif
    for (int i = 0; i < 6; ++i)
    {
        pills[i].setButtonText (names[i]);
        pills[i].setClickingTogglesState (false);
        pills[i].onClick = [this, i] { setMode (i); };
        addAndMakeVisible (pills[i]);
    }
    startTimerHz (12);
}
ModeBar::~ModeBar() { stopTimer(); }

void ModeBar::setMode (int index)
{
    if (auto* pm = proc.getAPVTS().getParameter (pid::mode))
        pm->setValueNotifyingHost ((float) index / 5.0f);
}

void ModeBar::timerCallback()
{
    const int cur = (int) proc.getAPVTS().getRawParameterValue (pid::mode)->load();
    for (int i = 0; i < 6; ++i)
        if (pills[i].getToggleState() != (i == cur))
            pills[i].setToggleState (i == cur, juce::dontSendNotification);
    repaint();   // keep logo glitch + online dot animated
}

void ModeBar::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();

    // carbon header
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xFF0C0C1A), 0, 0,
                                             juce::Colour (0xFF05000F), 0, r.getHeight(), false));
    g.fillRect (r);
    g.setColour (cyan.withAlpha (0.5f));
    g.drawHorizontalLine (getHeight() - 1, 0.0f, r.getWidth());

    // real neon camel badge (if embedded)
    int textX = 16;
    if (logo.isValid())
    {
        const int badge = getHeight() - 8;
        g.drawImageWithin (logo, 4, 4, badge, badge, juce::RectanglePlacement::centred, false);
        textX = 4 + badge + 6;
    }

    // "MPC2077" chrome wordmark + glitch offset
    auto logoArea = juce::Rectangle<int> (textX, 8, 300, 40);
    const float gj = (std::sin (phase * 6.0f) > 0.9f) ? 3.0f : 0.0f;
    g.setFont (theme::hud (32.0f, true));
    g.setColour (pink.withAlpha (0.6f));
    g.drawText ("MPC2077", logoArea.translated ((int) gj, 0), juce::Justification::centredLeft, false);
    g.setColour (cyan.withAlpha (0.6f));
    g.drawText ("MPC2077", logoArea.translated ((int) -gj, 0), juce::Justification::centredLeft, false);
    g.setColour (theme::textHi);
    g.drawText ("MPC2077", logoArea, juce::Justification::centredLeft, false);

    g.setFont (theme::hud (10.0f, true));
    g.setColour (pink);
    g.drawText ("by DJBILBOX BEATS", textX + 2, 48, 260, 14, juce::Justification::centredLeft, false);

    // status top-right
    const float dotAlpha = 0.5f + 0.5f * std::sin (phase * 4.0f);
    g.setColour (juce::Colour (0xFF2BFF88).withAlpha (dotAlpha));
    g.fillEllipse (r.getRight() - 92.0f, 14.0f, 7.0f, 7.0f);
    g.setColour (cyan);
    g.setFont (theme::hud (10.0f, true));
    g.drawText ("ONLINE", (int) r.getRight() - 80, 10, 74, 14, juce::Justification::centredLeft, false);
    g.setColour (textDim);
    g.setFont (theme::hud (9.0f));
    g.drawText ("v2.0.77", (int) r.getRight() - 80, 26, 74, 12, juce::Justification::centredLeft, false);
}

void ModeBar::resized()
{
    // center the 6 pills
    const int pillW = 78, pillH = 26, gap = 6;
    const int totalW = 6 * pillW + 5 * gap;
    int x = (getWidth() - totalW) / 2 + 40;   // nudge right of logo
    const int y = (getHeight() - pillH) / 2;
    for (int i = 0; i < 6; ++i)
    {
        pills[i].setBounds (x, y, pillW, pillH);
        x += pillW + gap;
    }
}
