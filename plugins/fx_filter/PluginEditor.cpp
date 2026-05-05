#include "PluginEditor.h"

namespace t = lxr::theme;

namespace {
    const char* idCutoff     = "cutoff";
    const char* idReso       = "reso";
    const char* idDrive      = "drive";
    const char* idFilterType = "filter_type";
}

LxrFilterEditor::LxrFilterEditor (LxrFilterProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&laf);

    auto add = [this] (auto& uptr, auto* component) {
        uptr.reset (component);
        addAndMakeVisible (uptr.get());
    };

    add (kCutoff, new lxr::LabeledKnob (proc.apvts, idCutoff, "Cutoff"));
    add (kReso,   new lxr::LabeledKnob (proc.apvts, idReso,   "Reso"));
    add (kDrive,  new lxr::LabeledKnob (proc.apvts, idDrive,  "Drive"));

    // 7 firmware filter types - using shorthand labels to fit in segments.
    typeSeg.reset (new lxr::SegmentedChoice (proc.apvts, idFilterType,
        juce::StringArray { "LP", "HP", "BP", "uBP", "Notch", "Peak", "2P" }));
    addAndMakeVisible (typeSeg.get());

    addAndMakeVisible (filterHeader);

    setSize (580, 240);
}

LxrFilterEditor::~LxrFilterEditor()
{
    setLookAndFeel (nullptr);
}

void LxrFilterEditor::paint (juce::Graphics& g)
{
    g.fillAll (t::panelBg);
    g.setColour (t::sectionBg);
    g.fillRoundedRectangle (filterRect.toFloat(), 6.0f);
    g.setColour (t::sectionBorder);
    g.drawRoundedRectangle (filterRect.toFloat().reduced (0.5f), 6.0f, 1.0f);
}

void LxrFilterEditor::resized()
{
    auto bounds = getLocalBounds().reduced (t::windowPadding);
    filterRect = bounds;

    auto inner = bounds.reduced (t::sectionPadding);
    filterHeader.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
    inner.removeFromTop (8);

    // Three knobs in one row, evenly spaced
    auto knobRow = inner.removeFromTop (t::knobCellHeight);
    const int n = 3;
    const int cellW = knobRow.getWidth() / n;
    int x = knobRow.getX();
    for (auto* k : { kCutoff.get(), kReso.get(), kDrive.get() })
    {
        k->setBounds (x, knobRow.getY(), cellW, t::knobCellHeight);
        x += cellW;
    }

    inner.removeFromTop (8);
    typeSeg->setBounds (inner.removeFromTop (t::segHeight));
}
