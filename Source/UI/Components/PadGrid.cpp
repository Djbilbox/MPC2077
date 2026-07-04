#include "PadGrid.h"
#include "../../PluginProcessor.h"

using namespace mpc;
using namespace mpc::theme;

namespace { constexpr int kCols = 4, kRows = 4, kGap = 8; }

PadGrid::PadGrid (MPC2077AudioProcessor& p) : proc (p)
{
    startTimerHz (30);
}
PadGrid::~PadGrid() { stopTimer(); }

//==============================================================================
juce::Rectangle<float> PadGrid::padBounds (int index) const
{
    const int col = index % kCols;
    const int row = index / kCols;
    auto area = getLocalBounds().reduced (2);
    const float cw = (area.getWidth()  - (kCols - 1) * kGap) / (float) kCols;
    const float ch = (area.getHeight() - (kRows - 1) * kGap) / (float) kRows;
    return { area.getX() + col * (cw + kGap), area.getY() + row * (ch + kGap), cw, ch };
}

int PadGrid::padAt (juce::Point<int> pos) const
{
    for (int i = 0; i < kNumPads; ++i)
        if (padBounds (i).contains (pos.toFloat()))
            return i;
    return -1;
}

//==============================================================================
void PadGrid::paint (juce::Graphics& g)
{
    const int sel = proc.selectedPad.load();
    auto& eng = proc.getEngine();

    for (int i = 0; i < kNumPads; ++i)
    {
        auto b = padBounds (i);
        const float lvl = eng.getPadLevel (i);
        const float fl  = flash[(size_t) i];
        const bool isSel  = (i == sel);
        const bool isDrag = (i == dragPad);
        const float rad = 6.0f;

        // contact shadow
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillRoundedRectangle (b.translated (0.0f, 2.0f), rad);

        // glossy 3D beveled body: dark steel with a cyan sheen (top-left lit, bottom-right dark)
        juce::ColourGradient body (juce::Colour (0xFF1C2430), b.getX(), b.getY(),
                                   juce::Colour (0xFF07080D), b.getRight(), b.getBottom(), false);
        body.addColour (0.45, juce::Colour (0xFF12161E));
        g.setGradientFill (body);
        g.fillRoundedRectangle (b, rad);

        // level glow tint (cyan) + hit flash (pink), blended on top of the metal
        if (lvl > 0.01f)
        {
            g.setColour (cyan.withAlpha (juce::jlimit (0.0f, 0.55f, lvl * 0.8f)));
            g.fillRoundedRectangle (b, rad);
        }
        if (fl > 0.01f)
        {
            g.setColour (pink.withAlpha (fl * 0.8f));
            g.fillRoundedRectangle (b, rad);
        }

        // specular sheen (top strip)
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.fillRoundedRectangle (b.reduced (b.getWidth() * 0.10f, 0.0f).withHeight (b.getHeight() * 0.28f), rad * 0.6f);

        // chrome / accent border
        if (isDrag)      { g.setColour (yellow); g.drawRoundedRectangle (b, rad, 2.2f); }
        else if (isSel)  { g.setColour (pink.withAlpha (0.22f)); g.drawRoundedRectangle (b.expanded (2.0f), rad + 1.0f, 3.0f);
                           g.setColour (pink); g.drawRoundedRectangle (b, rad, 1.6f); }
        else             { g.setColour (cyan.withAlpha (0.28f)); g.drawRoundedRectangle (b, rad, 1.0f); }

        // number
        g.setFont (theme::hud (9.0f, true));
        g.setColour ((isSel ? pink : cyan).withAlpha (0.9f));
        g.drawText (juce::String (i + 1).paddedLeft ('0', 2),
                    b.reduced (5.0f).removeFromTop (12.0f).toNearestInt(),
                    juce::Justification::topLeft, false);

        // name
        g.setFont (theme::hud (9.5f, false));
        g.setColour (textHi.withAlpha (0.85f));
        g.drawText (eng.pad (i).name, b.reduced (4.0f).toNearestInt(),
                    juce::Justification::centredBottom, false);

        // user-sample marker
        if (eng.pad (i).isUserSample)
        {
            g.setColour (yellow);
            g.fillEllipse (b.getRight() - 9.0f, b.getY() + 5.0f, 4.0f, 4.0f);
        }
    }
}

//==============================================================================
void PadGrid::mouseDown (const juce::MouseEvent& e)
{
    const int idx = padAt (e.getPosition());
    if (idx < 0) return;

    proc.selectedPad.store (idx);
    auto b = padBounds (idx);
    const float vel = juce::jlimit (0.4f, 1.0f, 1.0f - (e.position.y - b.getY()) / b.getHeight() * 0.5f);
    proc.triggerPadFromUI (idx, vel);
    flash[(size_t) idx] = 1.0f;
    if (onSelectionChanged) onSelectionChanged();
    repaint();
}

void PadGrid::mouseDoubleClick (const juce::MouseEvent& e)
{
    const int idx = padAt (e.getPosition());
    if (idx < 0) return;
    proc.selectedPad.store (idx);

    chooser = std::make_unique<juce::FileChooser> ("Load sample into pad " + juce::String (idx + 1),
                                                   juce::File{}, "*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mp3");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this, idx] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult();
        if (f.existsAsFile()) loadFileIntoPad (idx, f);
    });
}

//==============================================================================
bool PadGrid::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
        if (f.endsWithIgnoreCase (".wav") || f.endsWithIgnoreCase (".aif") || f.endsWithIgnoreCase (".aiff")
            || f.endsWithIgnoreCase (".flac") || f.endsWithIgnoreCase (".ogg") || f.endsWithIgnoreCase (".mp3"))
            return true;
    return false;
}

void PadGrid::fileDragMove (const juce::StringArray&, int x, int y)
{
    const int idx = padAt ({ x, y });
    if (idx != dragPad) { dragPad = idx; repaint(); }
}

void PadGrid::filesDropped (const juce::StringArray& files, int x, int y)
{
    const int idx = padAt ({ x, y });
    dragPad = -1;
    if (idx >= 0 && files.size() > 0)
    {
        loadFileIntoPad (idx, juce::File (files[0]));
        proc.selectedPad.store (idx);
        if (onSelectionChanged) onSelectionChanged();
    }
    repaint();
}

void PadGrid::loadFileIntoPad (int pad, const juce::File& f)
{
    if (proc.getEngine().loadSampleIntoPad (pad, f))
    {
        proc.triggerPadFromUI (pad, 0.9f);
        flash[(size_t) pad] = 1.0f;
        repaint();
    }
}

//==============================================================================
void PadGrid::timerCallback()
{
    bool need = false;
    for (auto& f : flash) { if (f > 0.001f) { f *= 0.80f; need = true; } }
    // also repaint if any pad has level (for meter glow)
    for (int i = 0; i < kNumPads; ++i) if (proc.getEngine().getPadLevel (i) > 0.01f) { need = true; break; }
    if (need) repaint();
}
