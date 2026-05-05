#include "PluginEditor.h"

namespace t = lxr::theme;

namespace {
    const char* idShape  = "shape";
    const char* idOutput = "output";
}

LxrDistortionEditor::LxrDistortionEditor (LxrDistortionProcessor& p)
    : juce::AudioProcessorEditor (p), proc (p)
{
    setLookAndFeel (&laf);

    auto add = [this] (auto& uptr, auto* component) {
        uptr.reset (component);
        addAndMakeVisible (uptr.get());
    };

    add (kShape,  new lxr::LabeledKnob (proc.apvts, idShape,  "Shape"));
    add (kOutput, new lxr::LabeledKnob (proc.apvts, idOutput, "Output"));

    addAndMakeVisible (header);

    setSize (340, 200);
}

LxrDistortionEditor::~LxrDistortionEditor()
{
    setLookAndFeel (nullptr);
}

void LxrDistortionEditor::paint (juce::Graphics& g)
{
    g.fillAll (t::panelBg);
    g.setColour (t::sectionBg);
    g.fillRoundedRectangle (sectionRect.toFloat(), 6.0f);
    g.setColour (t::sectionBorder);
    g.drawRoundedRectangle (sectionRect.toFloat().reduced (0.5f), 6.0f, 1.0f);
}

void LxrDistortionEditor::resized()
{
    auto bounds = getLocalBounds().reduced (t::windowPadding);
    sectionRect = bounds;

    auto inner = bounds.reduced (t::sectionPadding);
    header.setBounds (inner.removeFromTop (t::sectionHeaderHeight));
    inner.removeFromTop (8);

    auto knobRow = inner.removeFromTop (t::knobCellHeight);
    const int cellW = knobRow.getWidth() / 2;
    kShape->setBounds  (knobRow.getX(),         knobRow.getY(), cellW, t::knobCellHeight);
    kOutput->setBounds (knobRow.getX() + cellW, knobRow.getY(), cellW, t::knobCellHeight);
}
