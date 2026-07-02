#pragma once

#include <JuceHeader.h>

namespace mpc::theme
{
    // --- cyberpunk palette (from the design brief) ---
    const juce::Colour bg       { 0xFF05000F };
    const juce::Colour panel    { 0xFF080015 };
    const juce::Colour panelLo  { 0xFF0A0015 };
    const juce::Colour panelHi  { 0xFF120A22 };
    const juce::Colour cyan     { 0xFF00F5FF };
    const juce::Colour pink     { 0xFFFF2D78 };
    const juce::Colour yellow   { 0xFFFFE600 };
    const juce::Colour textHi   { 0xFFE8FBFF };
    const juce::Colour textDim  { 0xFF6A6A88 };
    const juce::Colour line     { 0x2200F5FF };

    inline juce::Font hud (float height, bool bold = false)
    {
        auto o = juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), height,
                                    bold ? juce::Font::bold : juce::Font::plain);
        return juce::Font (o);
    }

    /** Soft neon glow: several expanding strokes of decreasing alpha. */
    inline void glowPath (juce::Graphics& g, const juce::Path& p, juce::Colour c,
                          float baseWidth, float intensity)
    {
        for (int i = 3; i >= 1; --i)
        {
            g.setColour (c.withAlpha (0.12f * intensity * (float) i));
            g.strokePath (p, juce::PathStrokeType (baseWidth + (float) i * 2.5f,
                                                   juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
        }
        g.setColour (c.withAlpha (juce::jlimit (0.0f, 1.0f, 0.85f * intensity)));
        g.strokePath (p, juce::PathStrokeType (baseWidth));
    }

    inline void glowText (juce::Graphics& g, const juce::String& text, juce::Rectangle<int> area,
                          juce::Colour c, juce::Justification just, float glow = 0.5f)
    {
        g.setColour (c.withAlpha (0.30f * glow));
        for (auto d : { juce::Point<int> (-1, 0), { 1, 0 }, { 0, -1 }, { 0, 1 } })
            g.drawText (text, area.translated (d.x, d.y), just, false);
        g.setColour (c);
        g.drawText (text, area, just, false);
    }

    /** Draw a faint circuit-board / grid backdrop into a rectangle. */
    inline void circuitGrid (juce::Graphics& g, juce::Rectangle<float> r, float cell = 26.0f)
    {
        g.setColour (line);
        for (float x = r.getX(); x < r.getRight(); x += cell)
            g.drawVerticalLine ((int) x, r.getY(), r.getBottom());
        for (float y = r.getY(); y < r.getBottom(); y += cell)
            g.drawHorizontalLine ((int) y, r.getX(), r.getRight());
    }
}
