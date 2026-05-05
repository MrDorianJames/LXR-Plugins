#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pid {
    constexpr const char* oscPitch      = "osc_pitch";
    constexpr const char* noiseFilt     = "noise_filter";
    constexpr const char* mix           = "mix";
    constexpr const char* pitchEgAmt    = "pitch_eg_amount";
    constexpr const char* pitchDecay    = "pitch_decay";
    constexpr const char* volAttack     = "vol_attack";
    constexpr const char* volDecay      = "vol_decay";
    constexpr const char* volSlope      = "vol_slope";
    constexpr const char* transientAmt  = "transient_amount";
    constexpr const char* transientWave = "transient_wave";
    constexpr const char* snap          = "snap";
    constexpr const char* reso          = "reso";
    constexpr const char* filterType    = "filter_type";
    constexpr const char* distortion    = "distortion";
    constexpr const char* volume        = "volume";
    constexpr const char* pan           = "pan";
}

using P  = juce::AudioParameterFloat;
using PI = juce::AudioParameterInt;
using PC = juce::AudioParameterChoice;

juce::AudioProcessorValueTreeState::ParameterLayout
LxrSnareProcessor::buildParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto unit    = juce::NormalisableRange<float> (0.0f, 1.0f);
    auto bipolar = juce::NormalisableRange<float> (-1.0f, 1.0f);

    layout.add (std::make_unique<P>  (juce::ParameterID (pid::oscPitch,      1), "Osc Pitch",        unit,    0.50f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::noiseFilt,     1), "Cutoff",           unit,    0.70f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::mix,           1), "Mix (Osc/Noise)",  unit,    0.50f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pitchEgAmt,    1), "Pitch EG Amount",  bipolar, 0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pitchDecay,    1), "Pitch Decay",      unit,    0.30f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volAttack,     1), "Volume Attack",    unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volDecay,      1), "Volume Decay",     unit,    0.40f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volSlope,      1), "Volume Slope",     unit,    0.50f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::transientAmt,  1), "Transient Amount", unit,    0.50f));
    layout.add (std::make_unique<PI> (juce::ParameterID (pid::transientWave, 1), "Transient Wave",   0, 7,    0));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::snap,          1), "Snap",             unit,    0.20f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::reso,          1), "Resonance",        unit,    0.0f));
    layout.add (std::make_unique<PC> (juce::ParameterID (pid::filterType,    1), "Filter Type",
                                      juce::StringArray { "Off", "LP", "HP", "BP" }, 1));   // default LP
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::distortion,    1), "Distortion",       unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volume,        1), "Volume",           unit,    0.80f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pan,           1), "Pan",              unit,    0.50f));

    return layout;
}

LxrSnareProcessor::LxrSnareProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", buildParameterLayout())
{
    pOscPitch       = apvts.getRawParameterValue (pid::oscPitch);
    pNoiseFilt      = apvts.getRawParameterValue (pid::noiseFilt);
    pMix            = apvts.getRawParameterValue (pid::mix);
    pPitchEgAmt     = apvts.getRawParameterValue (pid::pitchEgAmt);
    pPitchDecay     = apvts.getRawParameterValue (pid::pitchDecay);
    pVolAttack      = apvts.getRawParameterValue (pid::volAttack);
    pVolDecay       = apvts.getRawParameterValue (pid::volDecay);
    pVolSlope       = apvts.getRawParameterValue (pid::volSlope);
    pTransientAmt   = apvts.getRawParameterValue (pid::transientAmt);
    pTransientWave  = apvts.getRawParameterValue (pid::transientWave);
    pSnap           = apvts.getRawParameterValue (pid::snap);
    pReso           = apvts.getRawParameterValue (pid::reso);
    pFilterType     = apvts.getRawParameterValue (pid::filterType);
    pDistortion     = apvts.getRawParameterValue (pid::distortion);
    pVolume         = apvts.getRawParameterValue (pid::volume);
    pPan            = apvts.getRawParameterValue (pid::pan);
}

void LxrSnareProcessor::prepareToPlay (double sr, int) { engine.prepare (sr); pushParametersToEngine(); }

bool LxrSnareProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto out = l.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void LxrSnareProcessor::pushParametersToEngine() noexcept
{
    engine.setOscPitch      (pOscPitch     ->load());
    engine.setNoiseFilter   (pNoiseFilt    ->load());
    engine.setMix           (pMix          ->load());
    engine.setPitchEgAmount (pPitchEgAmt   ->load());
    engine.setPitchDecay    (pPitchDecay   ->load());
    engine.setVolEgAttack   (pVolAttack    ->load());
    engine.setVolEgDecay    (pVolDecay     ->load());
    engine.setVolEgSlope    (pVolSlope     ->load());
    engine.setTransientAmt  (pTransientAmt ->load());
    engine.setTransientWave ((int) pTransientWave->load());
    engine.setSnap          (pSnap         ->load());
    engine.setReso          (pReso         ->load());
    engine.setFilterType    ((int) pFilterType->load());
    engine.setDistortion    (pDistortion   ->load());
    engine.setVolume        (pVolume       ->load());
    engine.setPan           (pPan          ->load());
}

void LxrSnareProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals nd;
    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    pushParametersToEngine();

    if (auto* head = getPlayHead())
        if (auto info = head->getPosition())
            if (auto bpm = info->getBpm())
                engine.setHostBpm (static_cast<float> (*bpm));

    float* mono = buffer.getWritePointer (0);
    int cursor = 0;
    for (auto meta : midi)
    {
        const auto m = meta.getMessage();
        const int  pos = (int) meta.samplePosition;
        if (pos > cursor) { engine.process (mono + cursor, pos - cursor); cursor = pos; }
        if (m.isNoteOn())
            engine.trigger ((uint8_t) m.getVelocity(), (uint8_t) m.getNoteNumber());
    }
    if (cursor < numSamples) engine.process (mono + cursor, numSamples - cursor);

    for (int ch = 1; ch < numCh; ++ch)
        buffer.copyFrom (ch, 0, buffer, 0, 0, numSamples);
}

juce::AudioProcessorEditor* LxrSnareProcessor::createEditor()
{
    return new LxrSnareEditor (*this);
}

void LxrSnareProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::MemoryOutputStream mos (dest, false);
    apvts.state.writeToStream (mos);
}

void LxrSnareProcessor::setStateInformation (const void* data, int size)
{
    auto t = juce::ValueTree::readFromData (data, (size_t) size);
    if (t.isValid()) apvts.replaceState (t);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LxrSnareProcessor();
}
