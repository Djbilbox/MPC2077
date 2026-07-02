#include "TouchRibbon.h"
#include "../../PluginProcessor.h"

using namespace mpc;
using namespace mpc::theme;

namespace { constexpr int kSideW = 70; }

TouchRibbon::TouchRibbon (MPC2077AudioProcessor& p) : proc (p)
{
    pitchSlider.setSliderStyle (juce::Slider::LinearVertical);
    pitchSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    pitchSlider.onDragEnd = [this] { pitchSlider.setValue (0.0, juce::sendNotificationSync); };
    addAndMakeVisible (pitchSlider);
    pitchAttach = std::make_unique<Attach> (proc.getAPVTS(), pid::pitchWheel, pitchSlider);

    modSlider.setSliderStyle (juce::Slider::LinearVertical);
    modSlider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible (modSlider);
    modAttach = std::make_unique<Attach> (proc.getAPVTS(), pid::modWheel, modSlider);
}

juce::Rectangle<int> TouchRibbon::ribbonBounds() const
{
    return getLocalBounds().reduced (kSideW + 8, 22).withTrimmedTop (2);
}

void TouchRibbon::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (panel);
    g.fillRect (r);
    g.setColour (cyan.withAlpha (0.4f));
    g.drawHorizontalLine (0, 0.0f, r.getWidth());

    // labels
    g.setFont (theme::hud (10.0f, true));
    g.setColour (cyan);
    g.drawText ("PITCH", 6, 4, kSideW, 14, juce::Justification::centred, false);
    g.setColour (pink);
    g.drawText ("MOD", getWidth() - kSideW - 6, 4, kSideW, 14, juce::Justification::centred, false);

    // ribbon
    auto rb = ribbonBounds();
    g.setColour (panelLo);
    g.fillRoundedRectangle (rb.toFloat(), 5.0f);
    g.setGradientFill (juce::ColourGradient (cyan.withAlpha (0.35f), rb.getX(), 0,
                                             pink.withAlpha (0.35f), rb.getRight(), 0, false));
    g.fillRoundedRectangle (rb.toFloat().reduced (2.0f), 4.0f);

    // key divisions (C2..C7-ish)
    g.setColour (cyan.withAlpha (0.20f));
    for (int i = 1; i < 12; ++i)
    {
        const float x = rb.getX() + rb.getWidth() * i / 12.0f;
        g.drawVerticalLine ((int) x, (float) rb.getY(), (float) rb.getBottom());
    }
    g.setColour (cyan.withAlpha (0.55f));
    g.setFont (theme::hud (8.0f));
    g.drawText ("C2", rb.getX() + 3, rb.getY(), 24, rb.getHeight(), juce::Justification::centredLeft, false);
    g.drawText ("C7", rb.getRight() - 27, rb.getY(), 24, rb.getHeight(), juce::Justification::centredRight, false);

    // touch highlight
    if (touchX >= 0.0f)
    {
        g.setColour (yellow.withAlpha (0.9f));
        g.fillRoundedRectangle (touchX - 3.0f, (float) rb.getY(), 6.0f, (float) rb.getHeight(), 3.0f);
        g.setColour (yellow.withAlpha (0.3f));
        g.fillRoundedRectangle (touchX - 8.0f, (float) rb.getY(), 16.0f, (float) rb.getHeight(), 5.0f);
    }
}

void TouchRibbon::playAt (juce::Point<int> pos)
{
    auto rb = ribbonBounds();
    const float t = juce::jlimit (0.0f, 1.0f, (float) (pos.x - rb.getX()) / (float) rb.getWidth());
    const int semis = juce::roundToInt (t * 48.0f - 24.0f);   // -24..+24
    if (semis != lastSemis)
    {
        proc.triggerPadFromUI (proc.selectedPad.load(), 0.9f, (float) semis);
        lastSemis = semis;
    }
    touchX = (float) juce::jlimit (rb.getX(), rb.getRight(), pos.x);
    repaint();
}

void TouchRibbon::mouseDown (const juce::MouseEvent& e)
{
    if (ribbonBounds().contains (e.getPosition())) { lastSemis = -1000; playAt (e.getPosition()); }
}
void TouchRibbon::mouseDrag (const juce::MouseEvent& e)
{
    if (ribbonBounds().contains (e.getPosition())) playAt (e.getPosition());
}
void TouchRibbon::mouseUp (const juce::MouseEvent&)
{
    touchX = -1.0f; lastSemis = -1000; repaint();
}

void TouchRibbon::resized()
{
    pitchSlider.setBounds (18, 20, kSideW - 30, getHeight() - 26);
    modSlider.setBounds (getWidth() - kSideW + 12, 20, kSideW - 30, getHeight() - 26);
}
