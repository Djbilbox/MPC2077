#include "ControlSection.h"
#include "../../PluginProcessor.h"

using namespace mpc;
using namespace mpc::theme;

ControlSection::ControlSection (MPC2077AudioProcessor& p, const juce::String& t)
    : proc (p), title (t) {}

void ControlSection::addParamKnob (const juce::String& label, const char* paramId)
{
    auto* k = new Knob();
    k->label = label;
    k->isPad = false;
    k->slider = std::make_unique<juce::Slider>();
    k->slider->setSliderStyle (juce::Slider::RotaryVerticalDrag);
    k->slider->setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    addAndMakeVisible (*k->slider);
    k->attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.getAPVTS(), paramId, *k->slider);
    knobs.add (k);
}

void ControlSection::addPadKnob (const juce::String& label, juce::NormalisableRange<double> range,
                                 std::function<double(const PadSettings&)> getter,
                                 std::function<void(PadSettings&, double)> setter)
{
    auto* k = new Knob();
    k->label = label;
    k->isPad = true;
    k->getter = getter;
    k->slider = std::make_unique<juce::Slider>();
    k->slider->setSliderStyle (juce::Slider::RotaryVerticalDrag);
    k->slider->setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    k->slider->setNormalisableRange (range);
    auto* s = k->slider.get();
    k->slider->onValueChange = [this, s, setter]
    {
        setter (proc.getEngine().pad (proc.selectedPad.load()), s->getValue());
    };
    addAndMakeVisible (*k->slider);
    knobs.add (k);
}

void ControlSection::refresh()
{
    const auto& pad = proc.getEngine().pad (proc.selectedPad.load());
    for (auto* k : knobs)
        if (k->isPad && k->getter)
            k->slider->setValue (k->getter (pad), juce::dontSendNotification);
    repaint();
}

void ControlSection::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (panel);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (cyan.withAlpha (0.30f));
    g.drawRoundedRectangle (r.reduced (0.8f), 6.0f, 1.0f);

    glowText (g, title, getLocalBounds().removeFromTop (18).reduced (6, 2),
              cyan, juce::Justification::centredLeft, 0.55f);

    g.setFont (theme::hud (8.5f, true));
    g.setColour (textHi.withAlpha (0.9f));
    for (auto* k : knobs)
    {
        auto kb = k->slider->getBounds();
        g.drawText (k->label, kb.getX() - 6, kb.getBottom() - 1, kb.getWidth() + 12, 13,
                    juce::Justification::centred, false);
    }
}

void ControlSection::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    area.removeFromTop (16);          // title
    const int n = juce::jmax (1, knobs.size());
    const int cw = area.getWidth() / n;
    for (int i = 0; i < knobs.size(); ++i)
    {
        juce::Rectangle<int> cell (area.getX() + i * cw, area.getY(), cw, area.getHeight());
        knobs[i]->slider->setBounds (cell.reduced (4).withTrimmedBottom (12));
    }
}
