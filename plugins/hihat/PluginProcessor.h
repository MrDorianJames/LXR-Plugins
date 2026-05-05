#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "HiHatEngine.h"

class LxrHiHatProcessor : public juce::AudioProcessor
{
public:
    LxrHiHatProcessor();
    ~LxrHiHatProcessor() override = default;

    void prepareToPlay (double sampleRate, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool   hasEditor() const override          { return true; }
    const juce::String getName() const override{ return "LxrHiHat"; }
    bool   acceptsMidi()  const override       { return true;  }
    bool   producesMidi() const override       { return false; }
    bool   isMidiEffect() const override       { return false; }
    double getTailLengthSeconds() const override { return 3.0; }

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

    lxr::HiHatEngine engine;

    std::atomic<float>* pMode          { nullptr };
    std::atomic<float>* pSplitNote     { nullptr };
    std::atomic<float>* pPitch         { nullptr };
    std::atomic<float>* pWaveform      { nullptr };
    std::atomic<float>* pMod1Pitch     { nullptr };
    std::atomic<float>* pMod1Amount    { nullptr };
    std::atomic<float>* pMod1Waveform  { nullptr };
    std::atomic<float>* pMod2Pitch     { nullptr };
    std::atomic<float>* pMod2Amount    { nullptr };
    std::atomic<float>* pMod2Waveform  { nullptr };
    std::atomic<float>* pDecayClosed   { nullptr };
    std::atomic<float>* pDecayOpen     { nullptr };
    std::atomic<float>* pVolAttack     { nullptr };
    std::atomic<float>* pVolSlope      { nullptr };
    std::atomic<float>* pTransientAmt  { nullptr };
    std::atomic<float>* pTransientWave { nullptr };
    std::atomic<float>* pSnap          { nullptr };
    std::atomic<float>* pCutoff        { nullptr };
    std::atomic<float>* pReso          { nullptr };
    std::atomic<float>* pFilterType    { nullptr };
    std::atomic<float>* pDistortion    { nullptr };
    std::atomic<float>* pVolume        { nullptr };
    std::atomic<float>* pPan           { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrHiHatProcessor)
};
