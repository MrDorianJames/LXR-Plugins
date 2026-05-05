#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pid {
    constexpr const char* shape  = "shape";
    constexpr const char* output = "output";
}

using P = juce::AudioParameterFloat;

juce::AudioProcessorValueTreeState::ParameterLayout
LxrDistortionProcessor::buildParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto unit = juce::NormalisableRange<float> (0.0f, 1.0f);

    layout.add (std::make_unique<P> (juce::ParameterID (pid::shape,  1), "Shape",  unit, 0.0f));
    layout.add (std::make_unique<P> (juce::ParameterID (pid::output, 1), "Output", unit, 0.5f));

    return layout;
}

LxrDistortionProcessor::LxrDistortionProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", buildParameterLayout())
{
    pShape  = apvts.getRawParameterValue (pid::shape);
    pOutput = apvts.getRawParameterValue (pid::output);
}

void LxrDistortionProcessor::prepareToPlay (double sr, int)
{
    engine.prepare (sr);
    pushParametersToEngine();
}

bool LxrDistortionProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto in  = layouts.getMainInputChannelSet();
    auto out = layouts.getMainOutputChannelSet();
    if (in != out) return false;
    return in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo();
}

void LxrDistortionProcessor::pushParametersToEngine() noexcept
{
    engine.setShape  (pShape ->load());
    engine.setOutput (pOutput->load());
}

void LxrDistortionProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals nd;
    pushParametersToEngine();

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    // The waveshaper is stateless, so we can run each channel
    // independently with the same DSP and they don't drift apart.
    for (int ch = 0; ch < numCh; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        engine.process (data, data, numSamples);
    }
}

juce::AudioProcessorEditor* LxrDistortionProcessor::createEditor()
{
    return new LxrDistortionEditor (*this);
}

void LxrDistortionProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::MemoryOutputStream mos (dest, false);
    apvts.state.writeToStream (mos);
}

void LxrDistortionProcessor::setStateInformation (const void* data, int size)
{
    auto t = juce::ValueTree::readFromData (data, (size_t) size);
    if (t.isValid()) apvts.replaceState (t);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LxrDistortionProcessor();
}
