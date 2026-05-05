#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DistortionEngine.h"

class LxrDistortionProcessor : public juce::AudioProcessor
{
public:
    LxrDistortionProcessor();
    ~LxrDistortionProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override                          {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool   hasEditor() const override          { return true; }

    const juce::String getName() const override{ return "LxrDistortion"; }
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

    lxr::DistortionEngine engine;

    std::atomic<float>* pShape  { nullptr };
    std::atomic<float>* pOutput { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrDistortionProcessor)
};
