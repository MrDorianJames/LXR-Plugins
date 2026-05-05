#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DrumEngine.h"

class LxrDrumProcessor : public juce::AudioProcessor
{
public:
    LxrDrumProcessor();
    ~LxrDrumProcessor() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override                          {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool                        hasEditor() const override     { return true; }

    const juce::String getName() const override                { return "LxrDrum"; }
    bool   acceptsMidi()                  const override       { return true;  }
    bool   producesMidi()                 const override       { return false; }
    bool   isMidiEffect()                 const override       { return false; }
    double getTailLengthSeconds()         const override       { return 2.0; }

    int    getNumPrograms()               override             { return 1; }
    int    getCurrentProgram()            override             { return 0; }
    void   setCurrentProgram (int)        override             {}
    const juce::String getProgramName (int) override           { return {}; }
    void   changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& dest) override;
    void setStateInformation (const void* data, int size) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout buildParameterLayout();
    void pushParametersToEngine() noexcept;

    lxr::DrumEngine engine;

    // Cached atomic pointers into the APVTS so the audio thread doesn't
    // hit the parameter map by string each block.
    std::atomic<float>* pPitch          { nullptr };
    std::atomic<float>* pWaveform       { nullptr };
    std::atomic<float>* pFmAmount       { nullptr };
    std::atomic<float>* pModPitch       { nullptr };
    std::atomic<float>* pPitchEgAmt     { nullptr };
    std::atomic<float>* pPitchDecay     { nullptr };
    std::atomic<float>* pVolAttack      { nullptr };
    std::atomic<float>* pVolDecay       { nullptr };
    std::atomic<float>* pVolSlope       { nullptr };
    std::atomic<float>* pTransientAmt   { nullptr };
    std::atomic<float>* pTransientWave  { nullptr };
    std::atomic<float>* pSnap           { nullptr };
    std::atomic<float>* pCutoff         { nullptr };
    std::atomic<float>* pReso           { nullptr };
    std::atomic<float>* pFilterType     { nullptr };
    std::atomic<float>* pDistortion     { nullptr };
    std::atomic<float>* pVolume         { nullptr };
    std::atomic<float>* pPan            { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LxrDrumProcessor)
};
