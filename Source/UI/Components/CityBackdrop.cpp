#include "CityBackdrop.h"

using namespace mpc;
using namespace mpc::theme;

CityBackdrop::CityBackdrop()
{
    setInterceptsMouseClicks (false, false);
}

void CityBackdrop::resized()
{
    if (getWidth() <= 0 || getHeight() <= 0) return;

    cache = juce::Image (juce::Image::ARGB, getWidth(), getHeight(), true);
    juce::Graphics g (cache);
    auto r = getLocalBounds().toFloat();

    // deep gradient
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xFF0A0A1E), r.getCentreX(), r.getY(),
                                             juce::Colour (0xFF04000C), r.getCentreX(), r.getBottom(), false));
    g.fillRect (r);
    circuitGrid (g, r, 30.0f);

    // horizon glow
    const float hz = r.getY() + r.getHeight() * 0.66f;
    g.setGradientFill (juce::ColourGradient (pink.withAlpha (0.16f), r.getCentreX(), hz,
                                             juce::Colours::transparentBlack, r.getCentreX(), hz - 90.0f, false));
    g.fillRect (r.withTop (hz - 90.0f).withBottom (hz));

    // buildings + windows (static)
    juce::Random rr (0x2077);
    float x = r.getX();
    while (x < r.getRight())
    {
        const float bw = 22.0f + rr.nextFloat() * 40.0f;
        const float bh = 60.0f + rr.nextFloat() * 190.0f;
        juce::Rectangle<float> b (x, hz - bh, bw - 4.0f, bh);
        g.setColour (juce::Colour (0xFF0B0B22).withAlpha (0.92f));
        g.fillRect (b);
        g.setColour (cyan.withAlpha (0.15f));
        g.drawRect (b, 0.8f);
        for (float wy = b.getY() + 6.0f; wy < b.getBottom() - 4.0f; wy += 10.0f)
            for (float wx = b.getX() + 4.0f; wx < b.getRight() - 4.0f; wx += 8.0f)
            {
                const float lit = rr.nextFloat();
                if (lit > 0.55f)
                {
                    auto wc = (lit > 0.85f ? pink : cyan);
                    g.setColour (wc.withAlpha (0.25f + 0.35f * lit));
                    g.fillRect (wx, wy, 3.0f, 4.0f);
                }
            }
        x += bw;
    }

    // vignette
    g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, r.getCentreX(), r.getCentreY(),
                                             juce::Colours::black.withAlpha (0.55f), r.getX(), r.getY(), true));
    g.fillRect (r);
}

void CityBackdrop::paint (juce::Graphics& g)
{
    if (cache.isValid())
        g.drawImageAt (cache, 0, 0);
    else
        g.fillAll (juce::Colour (0xFF04000C));

    auto r = getLocalBounds().toFloat();

    // animated rain (cheap)
    g.setColour (cyan.withAlpha (0.05f));
    for (int i = 0; i < 50; ++i)
    {
        const float rx = r.getX() + rng.nextFloat() * r.getWidth();
        const float ry = std::fmod (rng.nextFloat() * r.getHeight() + phase * 60.0f, r.getHeight());
        g.drawLine (rx, ry, rx - 2.0f, ry + 12.0f, 1.0f);
    }

    // glitchy MPC2077 watermark
    const juce::String wm ("MPC2077");
    auto wmArea = r.reduced (18.0f).withTrimmedTop (r.getHeight() * 0.30f)
                                   .withTrimmedBottom (r.getHeight() * 0.32f).toNearestInt();
    g.setFont (theme::hud (96.0f, true));
    const float gj = (std::sin (phase * 7.3f) > 0.93f) ? 4.0f : 0.0f;
    g.setColour (pink.withAlpha (0.10f));
    g.drawText (wm, wmArea.translated ((int) gj, 0), juce::Justification::centred, false);
    g.setColour (cyan.withAlpha (0.10f));
    g.drawText (wm, wmArea.translated ((int) -gj, 0), juce::Justification::centred, false);
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawText (wm, wmArea, juce::Justification::centred, false);
}
