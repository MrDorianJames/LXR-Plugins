#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pid {
    constexpr const char* cutoff     = "cutoff";
    constexpr const char* reso       = "reso";
    constexpr const char* drive      = "drive";
    constexpr const char* filterType = "filter_type";
}

using P  = juce::AudioParameterFloat;
using PC = juce::AudioParameterChoice;

juce::AudioProcessorValueTreeState::ParameterLayout
LxrFilterProcessor::buildParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto unit = juce::NormalisableRange<float> (0.0f, 1.0f);

    layout.add (std::make_unique<P>  (juce::ParameterID (pid::cutoff,     1), "Cutoff",      unit,    1.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::reso,       1), "Resonance",   unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::drive,      1), "Drive",       unit,    0.0f));
    layout.add (std::make_unique<PC> (juce::ParameterID (pid::filterType, 1), "Type",
                                      juce::StringArray {
                                          "LP", "HP", "BP", "Unity BP", "Notch", "Peak", "2-Pole LP"
                                      }, 0));   // default LP

    return layout;
}

LxrFilterProcessor::LxrFilterProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", buildParameterLayout())
{
    pCutoff     = apvts.getRawParameterValue (pid::cutoff);
    pReso       = apvts.getRawParameterValue (pid::reso);
    pDrive      = apvts.getRawParameterValue (pid::drive);
    pFilterType = apvts.getRawParameterValue (pid::filterType);
}

void LxrFilterProcessor::prepareToPlay (double sr, int blockSize)
{
    engine.prepare (sr, blockSize);
    pushParametersToEngine();
}

bool LxrFilterProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto in  = layouts.getMainInputChannelSet();
    auto out = layouts.getMainOutputChannelSet();
    if (in != out) return false;
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
}

void LxrFilterProcessor::pushParametersToEngine() noexcept
{
    engine.setCutoff     (pCutoff    ->load());
    engine.setResonance  (pReso      ->load());
    engine.setDrive      (pDrive     ->load());
    // The choice index is 0..6; the engine wants 1..7 (LXR enum).
    engine.setFilterType (1 + (int) pFilterType->load());
}

void LxrFilterProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    pushParametersToEngine();

    const int numSamples = buffer.getNumSamples();
    const int numCh      = std::min (buffer.getNumChannels(), 2);

    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        engine.process (data, data, numSamples, ch);
    }
}

juce::AudioProcessorEditor* LxrFilterProcessor::createEditor()
{
    return new LxrFilterEditor (*this);
}

void LxrFilterProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::MemoryOutputStream mos (dest, false);
    apvts.state.writeToStream (mos);
}

void LxrFilterProcessor::setStateInformation (const void* data, int size)
{
    auto t = juce::ValueTree::readFromData (data, (size_t) size);
    if (t.isValid()) apvts.replaceState (t);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LxrFilterProcessor();
}
