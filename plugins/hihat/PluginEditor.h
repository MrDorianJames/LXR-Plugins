#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "../shared/LxrLookAndFeel.h"
#include "../shared/Components.h"

class LxrHiHatEditor : public juce::AudioProcessorEditor
{
public:
    explicit LxrHiHatEditor (LxrHiHatProcessor&);
    ~LxrHiHatEditor() override;

    void paint   (juce::Graphics&) override;
    void resized() override;

private:
    LxrHiHatProcessor& proc;
    lxr::LxrLookAndFeel laf;

    lxr::SectionHeader triggerHeader { "Trigger" };
    lxr::SectionHeader carrierHeader { "Carrier" };
    lxr::SectionHeader fm1Header     { "FM 1" };
    lxr::SectionHeader fm2Header     { "FM 2" };
    lxr::SectionHeader envHeader     { "Volume Env" };
    lxr::SectionHeader transHeader   { "Transient" };
    lxr::SectionHeader filterHeader  { "Filter" };
    lxr::SectionHeader outputHeader  { "Output" };

    std::unique_ptr<lxr::LabeledKnob> kSplitNote;
    std::unique_ptr<lxr::LabeledKnob> kPitch;
    std::unique_ptr<lxr::LabeledKnob> kFm1, kGain1;
    std::unique_ptr<lxr::LabeledKnob> kFm2, kGain2;
    std::unique_ptr<lxr::LabeledKnob> kAttack, kDecayClosed, kDecayOpen, kSlope;
    std::unique_ptr<lxr::LabeledKnob> kTransAmt, kTransWave, kSnap;
    std::unique_ptr<lxr::LabeledKnob> kCutoff, kReso;
    std::unique_ptr<lxr::LabeledKnob> kDist, kVol, kPan;

    std::unique_ptr<lxr::SegmentedChoice> modeSeg;
    std::unique_ptr<lxr::SegmentedChoice> waveformSeg;
    std::unique_ptr<lxr::SegmentedChoice> wave1Seg;
    std::unique_ptr<lxr::SegmentedChoice> wave2Seg;
    std::unique_ptr<lxr::SegmentedChoice> filterTypeSeg;

    juce::Rectangle<int> triggerRect, carrierRect, fm1Rect, fm2Rect;
    juce::Rectangle<int> envRect, transRect, filterRect, outputRect;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrHiHatEditor)
};
