#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "../shared/LxrLookAndFeel.h"
#include "../shared/Components.h"

class LxrDistortionEditor : public juce::AudioProcessorEditor
{
public:
    explicit LxrDistortionEditor (LxrDistortionProcessor&);
    ~LxrDistortionEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    LxrDistortionProcessor& proc;
    lxr::LxrLookAndFeel laf;

    lxr::SectionHeader header { "Distortion" };

    std::unique_ptr<lxr::LabeledKnob> kShape, kOutput;

    juce::Rectangle<int> sectionRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrDistortionEditor)
};
