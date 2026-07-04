#include "MPC2077LookAndFeel.h"

using namespace mpc;
using namespace mpc::theme;

MPC2077LookAndFeel::MPC2077LookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, cyan);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, textHi);
    setColour (juce::PopupMenu::backgroundColourId, panelLo);
    setColour (juce::PopupMenu::textColourId, cyan);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, pink.withAlpha (0.4f));
}

//==============================================================================
void MPC2077LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                           float sliderPos, float startAngle, float endAngle,
                                           juce::Slider&)
{
    // Minimal hardware-synth knob (APESHYT-style): flat dark disc, thin value arc,
    // small pointer line. No big neon halo — just a crisp cyan/pink accent.
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float r = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    // thin background track (full sweep)
    juce::Path track;
    track.addCentredArc (c.x, c.y, r, r, 0.0f, startAngle, endAngle, true);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.strokePath (track, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // pink value arc (thin, crisp — no glow bloom)
    juce::Path val;
    val.addCentredArc (c.x, c.y, r, r, 0.0f, startAngle, angle, true);
    g.setColour (pink);
    g.strokePath (val, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // flat dark disc body with subtle top-light / bottom-dark bevel
    const float dr = r * 0.62f;
    juce::ColourGradient bodyGrad (panelHi.brighter (0.04f), c.x, c.y - dr,
                                   juce::Colour (0xFF060010), c.x, c.y + dr, false);
    g.setGradientFill (bodyGrad);
    g.fillEllipse (c.x - dr, c.y - dr, dr * 2.0f, dr * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawEllipse (c.x - dr, c.y - dr, dr * 2.0f, dr * 2.0f, 1.0f);
    g.setColour (cyan.withAlpha (0.35f));
    g.drawEllipse (c.x - dr + 0.6f, c.y - dr + 0.6f, dr * 2.0f - 1.2f, dr * 2.0f - 1.2f, 0.7f);

    // small pointer line (cyan, crisp)
    juce::Path ptr;
    ptr.startNewSubPath (c.x + std::sin (angle) * dr * 0.30f, c.y - std::cos (angle) * dr * 0.30f);
    ptr.lineTo (c.x + std::sin (angle) * dr * 0.86f, c.y - std::cos (angle) * dr * 0.86f);
    g.setColour (cyan);
    g.strokePath (ptr, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

//==============================================================================
void MPC2077LookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h,
                                           float sliderPos, float, float,
                                           juce::Slider::SliderStyle style, juce::Slider&)
{
    if (style == juce::Slider::LinearVertical)
    {
        const float cx = (float) x + w * 0.5f;
        const float top = (float) y + 4.0f;
        const float bot = (float) y + h - 4.0f;

        // track
        g.setColour (panelLo);
        g.fillRoundedRectangle (cx - 3.0f, top, 6.0f, bot - top, 3.0f);

        // active cyan fill (from bottom up to thumb)
        juce::Path fill;
        fill.startNewSubPath (cx, bot);
        fill.lineTo (cx, sliderPos);
        glowPath (g, fill, cyan, 4.0f, 0.8f);

        // pink glowing thumb
        const float ty = sliderPos;
        g.setColour (pink.withAlpha (0.25f));
        g.fillRoundedRectangle (cx - 11.0f, ty - 7.0f, 22.0f, 14.0f, 4.0f);
        g.setColour (pink);
        g.fillRoundedRectangle (cx - 8.0f, ty - 4.5f, 16.0f, 9.0f, 3.0f);
        g.setColour (juce::Colours::white.withAlpha (0.7f));
        g.fillRoundedRectangle (cx - 6.0f, ty - 2.5f, 12.0f, 2.0f, 1.0f);
    }
    else // horizontal fallback
    {
        const float cy = (float) y + h * 0.5f;
        g.setColour (panelLo);
        g.fillRoundedRectangle ((float) x, cy - 3.0f, (float) w, 6.0f, 3.0f);
        juce::Path fill;
        fill.startNewSubPath ((float) x, cy);
        fill.lineTo (sliderPos, cy);
        glowPath (g, fill, cyan, 4.0f, 0.8f);
        g.setColour (pink);
        g.fillRoundedRectangle (sliderPos - 4.5f, cy - 8.0f, 9.0f, 16.0f, 3.0f);
    }
}

//==============================================================================
void MPC2077LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b,
                                               const juce::Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.5f);
    const bool on = b.getToggleState() || down;
    const float rad = juce::jmin (r.getHeight() * 0.5f, 14.0f);

    if (on)
    {
        g.setColour (cyan.withAlpha (0.22f));
        g.fillRoundedRectangle (r.expanded (2.0f), rad + 2.0f);
        g.setColour (cyan);
        g.fillRoundedRectangle (r, rad);
    }
    else
    {
        g.setColour (panelLo);
        g.fillRoundedRectangle (r, rad);
        g.setColour (highlighted ? cyan.withAlpha (0.6f) : cyan.withAlpha (0.30f));
        g.drawRoundedRectangle (r, rad, 1.2f);
    }
}

void MPC2077LookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down)
{
    const bool on = b.getToggleState() || down;
    g.setFont (theme::hud (12.0f, true));
    g.setColour (on ? juce::Colours::black : textDim);
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred, false);
}
