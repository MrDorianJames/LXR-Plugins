#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "../shared/LxrLookAndFeel.h"
#include "../shared/Components.h"

class LxrSnareEditor : public juce::AudioProcessorEditor
{
public:
    explicit LxrSnareEditor (LxrSnareProcessor&);
    ~LxrSnareEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    LxrSnareProcessor& proc;
    lxr::LxrLookAndFeel laf;

    lxr::SectionHeader oscHeader     { "Oscillator" };
    lxr::SectionHeader envHeader     { "Volume Env" };
    lxr::SectionHeader transHeader   { "Transient" };
    lxr::SectionHeader filterHeader  { "Filter" };
    lxr::SectionHeader outputHeader  { "Output" };

    std::unique_ptr<lxr::LabeledKnob> kPitch, kMix, kPitchEgAmt, kPitchDecay;
    std::unique_ptr<lxr::LabeledKnob> kAttack, kDecay, kSlope;
    std::unique_ptr<lxr::LabeledKnob> kTransAmt, kTransWave, kSnap;
    std::unique_ptr<lxr::LabeledKnob> kCutoff, kReso;
    std::unique_ptr<lxr::LabeledKnob> kDist, kVol, kPan;

    std::unique_ptr<lxr::SegmentedChoice> filterTypeSeg;

    juce::Rectangle<int> oscRect, envRect, transRect, filterRect, outputRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrSnareEditor)
};
