#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FilterEngine.h"

class LxrFilterProcessor : public juce::AudioProcessor
{
public:
    LxrFilterProcessor();
    ~LxrFilterProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override                          {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool   hasEditor() const override          { return true; }

    const juce::String getName() const override{ return "LxrFilter"; }
    bool   acceptsMidi()  const override       { return false; }
    bool   producesMidi() const override       { return false; }
    bool   isMidiEffect() const override       { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int    getNumPrograms()         override   { return 1; }
    int    getCurrentProgram()      override   { return 0; }
    void   setCurrentProgram (int)  override   {}
    const juce::String getProgramName (int) override { return {}; }
    void   changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void* data, int size) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout buildParameterLayout();
    void pushParametersToEngine() noexcept;

    lxr::FilterEngine engine;

    std::atomic<float>* pCutoff      { nullptr };
    std::atomic<float>* pReso        { nullptr };
    std::atomic<float>* pDrive       { nullptr };
    std::atomic<float>* pFilterType  { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrFilterProcessor)
};
