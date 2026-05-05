#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SnareEngine.h"

class LxrSnareProcessor : public juce::AudioProcessor
{
public:
    LxrSnareProcessor();
    ~LxrSnareProcessor() override = default;

    void prepareToPlay (double sampleRate, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "LxrSnare"; }
    bool   acceptsMidi()  const override        { return true; }
    bool   producesMidi() const override        { return false; }
    bool   isMidiEffect() const override        { return false; }
    double getTailLengthSeconds() const override{ return 2.0; }

    int    getNumPrograms()           override  { return 1; }
    int    getCurrentProgram()        override  { return 0; }
    void   setCurrentProgram (int)    override  {}
    const juce::String getProgramName (int) override { return {}; }
    void   changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void* data, int size) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout buildParameterLayout();
    void pushParametersToEngine() noexcept;

    lxr::SnareEngine engine;

    std::atomic<float>* pOscPitch     { nullptr };
    std::atomic<float>* pNoiseFilt    { nullptr };
    std::atomic<float>* pMix          { nullptr };
    std::atomic<float>* pPitchEgAmt   { nullptr };
    std::atomic<float>* pPitchDecay   { nullptr };
    std::atomic<float>* pVolAttack    { nullptr };
    std::atomic<float>* pVolDecay     { nullptr };
    std::atomic<float>* pVolSlope     { nullptr };
    std::atomic<float>* pTransientAmt { nullptr };
    std::atomic<float>* pTransientWave{ nullptr };
    std::atomic<float>* pSnap         { nullptr };
    std::atomic<float>* pReso         { nullptr };
    std::atomic<float>* pFilterType   { nullptr };
    std::atomic<float>* pDistortion   { nullptr };
    std::atomic<float>* pVolume       { nullptr };
    std::atomic<float>* pPan          { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrSnareProcessor)
};
