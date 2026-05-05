#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "../shared/LxrLookAndFeel.h"
#include "../shared/Components.h"

class LxrDrumEditor : public juce::AudioProcessorEditor
{
public:
    explicit LxrDrumEditor (LxrDrumProcessor&);
    ~LxrDrumEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    LxrDrumProcessor& proc;
    lxr::LxrLookAndFeel laf;

    // Section headers
    lxr::SectionHeader oscHeader     { "Oscillator" };
    lxr::SectionHeader envHeader     { "Volume Env" };
    lxr::SectionHeader transHeader   { "Transient" };
    lxr::SectionHeader filterHeader  { "Filter" };
    lxr::SectionHeader outputHeader  { "Output" };

    // Knobs (built in ctor with parameter IDs from PluginProcessor.cpp)
    std::unique_ptr<lxr::LabeledKnob> kPitch, kModPitch, kFmAmount;
    std::unique_ptr<lxr::LabeledKnob> kPitchEgAmt, kPitchDecay;
    std::unique_ptr<lxr::LabeledKnob> kAttack, kDecay, kSlope;
    std::unique_ptr<lxr::LabeledKnob> kTransAmt, kTransWave, kSnap;
    std::unique_ptr<lxr::LabeledKnob> kCutoff, kReso;
    std::unique_ptr<lxr::LabeledKnob> kDist, kVol, kPan;

    // Segmented choices
    std::unique_ptr<lxr::SegmentedChoice> waveformSeg;
    std::unique_ptr<lxr::SegmentedChoice> filterTypeSeg;

    // Section background drawing
    juce::Rectangle<int> oscRect, envRect, transRect, filterRect, outputRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrDrumEditor)
};
