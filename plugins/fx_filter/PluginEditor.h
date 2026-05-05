#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "../shared/LxrLookAndFeel.h"
#include "../shared/Components.h"

class LxrFilterEditor : public juce::AudioProcessorEditor
{
public:
    explicit LxrFilterEditor (LxrFilterProcessor&);
    ~LxrFilterEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    LxrFilterProcessor& proc;
    lxr::LxrLookAndFeel laf;

    lxr::SectionHeader filterHeader { "Filter" };

    std::unique_ptr<lxr::LabeledKnob> kCutoff, kReso, kDrive;
    std::unique_ptr<lxr::SegmentedChoice> typeSeg;

    juce::Rectangle<int> filterRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrFilterEditor)
};
