#pragma once

#include <JuceHeader.h>
#include "Theme.h"

namespace mpc
{
/** Custom neon look — holographic knobs, glowing sliders, HUD pills. */
class MPC2077LookAndFeel : public juce::LookAndFeel_V4
{
public:
    MPC2077LookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool highlighted, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;

    juce::Font getLabelFont (juce::Label&) override { return theme::hud (11.0f); }
};
}
