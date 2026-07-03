#include "SequencerView.h"
#include "../../PluginProcessor.h"

using namespace mpc;
using namespace mpc::theme;

namespace { constexpr int kTopStrip = 36; constexpr int kLabelW = 62; }

SequencerView::SequencerView (MPC2077AudioProcessor& p) : proc (p)
{
    stepsBtn.onClick = [this]
    {
        auto& seq = proc.getSequencer();
        seq.setNumSteps (seq.getNumSteps() >= 32 ? 16 : 32);
        stepsBtn.setButtonText (juce::String (seq.getNumSteps()));
        repaint();
    };
    clearBtn.onClick = [this] { proc.getSequencer().clearAll(); repaint(); };
    addAndMakeVisible (stepsBtn);
    addAndMakeVisible (clearBtn);
    stepsBtn.setButtonText (juce::String (proc.getSequencer().getNumSteps()));

    for (auto* s : { &swingSlider, &humSlider })
    {
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        addAndMakeVisible (s);
    }
    swingAttach = std::make_unique<Attach> (proc.getAPVTS(), pid::swing, swingSlider);
    humAttach   = std::make_unique<Attach> (proc.getAPVTS(), pid::humanize, humSlider);

    startTimerHz (30);
}
SequencerView::~SequencerView() { stopTimer(); }

juce::Rectangle<int> SequencerView::gridArea() const
{
    return getLocalBounds().withTrimmedTop (kTopStrip + 14).reduced (4, 6).withTrimmedLeft (kLabelW);
}

bool SequencerView::cellAt (juce::Point<int> pos, int& pad, int& step) const
{
    auto ga = gridArea();
    if (! ga.contains (pos)) return false;
    const int n = proc.getSequencer().getNumSteps();
    const float cw = ga.getWidth() / (float) n;
    const float ch = ga.getHeight() / (float) kNumPads;
    step = juce::jlimit (0, n - 1, (int) ((pos.x - ga.getX()) / cw));
    pad  = juce::jlimit (0, kNumPads - 1, (int) ((pos.y - ga.getY()) / ch));
    return true;
}

void SequencerView::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (panel.withAlpha (0.92f));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (cyan.withAlpha (0.4f));
    g.drawRoundedRectangle (r.reduced (1.0f), 6.0f, 1.2f);

    auto& seq = proc.getSequencer();
    const int n = seq.getNumSteps();
    const int cur = seq.getCurrentStep();
    auto ga = gridArea();
    const float cw = ga.getWidth() / (float) n;
    const float ch = ga.getHeight() / (float) kNumPads;

    // step-number header + beat groups
    g.setFont (theme::hud (8.0f, true));
    for (int s = 0; s < n; ++s)
    {
        const float x = ga.getX() + s * cw;
        const bool beat = (s % 4 == 0);
        if (s == cur)
        {
            g.setColour (yellow.withAlpha (0.18f));
            g.fillRect (x, (float) ga.getY(), cw, (float) ga.getHeight());
        }
        g.setColour ((beat ? cyan : textDim).withAlpha (0.8f));
        g.drawText (juce::String (s + 1), (int) x, ga.getY() - 14, (int) cw, 12, juce::Justification::centred, false);
    }

    // rows
    for (int pad = 0; pad < kNumPads; ++pad)
    {
        const float y = ga.getY() + pad * ch;
        const bool selRow = (pad == proc.selectedPad.load());

        // label
        g.setColour ((selRow ? pink : cyan).withAlpha (0.85f));
        g.setFont (theme::hud (9.0f, selRow));
        g.drawText (juce::String (pad + 1).paddedLeft ('0', 2) + " " + proc.getEngine().pad (pad).name,
                    2, (int) y, kLabelW - 4, (int) ch, juce::Justification::centredLeft, false);

        for (int s = 0; s < n; ++s)
        {
            const float x = ga.getX() + s * cw;
            juce::Rectangle<float> cell (x + 1.0f, y + 1.0f, cw - 2.0f, ch - 2.0f);
            const bool on = seq.getStep (pad, s);
            const bool beat = (s % 4 == 0);

            g.setColour (beat ? panelHi : panelLo);
            g.fillRoundedRectangle (cell, 2.0f);

            if (on)
            {
                auto c = (s == cur) ? pink : cyan;
                g.setColour (c.withAlpha (0.9f));
                g.fillRoundedRectangle (cell, 2.0f);
                g.setColour (juce::Colours::white.withAlpha (0.5f));
                g.fillRoundedRectangle (cell.reduced (cell.getWidth() * 0.32f, cell.getHeight() * 0.32f), 1.0f);
            }
            else
            {
                g.setColour (cyan.withAlpha (0.12f));
                g.drawRoundedRectangle (cell, 2.0f, 0.6f);
            }
        }
    }

    // control labels
    g.setColour (cyan.withAlpha (0.8f));
    g.setFont (theme::hud (8.5f, true));
    g.drawText ("SWING", swingSlider.getX(), swingSlider.getY() - 12, swingSlider.getWidth(), 11, juce::Justification::centred, false);
    g.drawText ("HUMANIZE", humSlider.getX(), humSlider.getY() - 12, humSlider.getWidth(), 11, juce::Justification::centred, false);
}

void SequencerView::mouseDown (const juce::MouseEvent& e)
{
    int pad, step;
    if (! cellAt (e.getPosition(), pad, step)) return;
    proc.selectedPad.store (pad);
    paintValue = ! proc.getSequencer().getStep (pad, step);
    proc.getSequencer().setStep (pad, step, paintValue);
    lastStep = step * 100 + pad;
    repaint();
}

void SequencerView::mouseDrag (const juce::MouseEvent& e)
{
    int pad, step;
    if (! cellAt (e.getPosition(), pad, step)) return;
    const int key = step * 100 + pad;
    if (key == lastStep) return;
    proc.getSequencer().setStep (pad, step, paintValue);
    lastStep = key;
    repaint();
}

void SequencerView::timerCallback()
{
    if (proc.getSequencer().isRunning()) repaint();
}

void SequencerView::resized()
{
    // top strip: [SWING][HUMANIZE] .... [16/32][CLR]
    auto top = getLocalBounds().removeFromTop (kTopStrip).reduced (6, 0);
    auto right = top.removeFromRight (90).withTrimmedTop (7).withTrimmedBottom (5);
    stepsBtn.setBounds (right.removeFromLeft (42).reduced (2));
    clearBtn.setBounds (right.removeFromLeft (42).reduced (2));

    // sliders sit in the lower half of the top strip; labels are painted above them
    auto sl = top.withTrimmedTop (16).withTrimmedBottom (4).withTrimmedRight (16);
    const int half = sl.getWidth() / 2;
    swingSlider.setBounds (sl.removeFromLeft (half).reduced (6, 0));
    humSlider.setBounds (sl.reduced (6, 0));
}
