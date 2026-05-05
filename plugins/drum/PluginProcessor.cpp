#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pid {
    constexpr const char* pitch         = "pitch";
    constexpr const char* waveform      = "waveform";
    constexpr const char* fmAmount      = "fm_amount";
    constexpr const char* modPitch      = "mod_pitch";
    constexpr const char* pitchEgAmt    = "pitch_eg_amount";
    constexpr const char* pitchDecay    = "pitch_decay";
    constexpr const char* volAttack     = "vol_attack";
    constexpr const char* volDecay      = "vol_decay";
    constexpr const char* volSlope      = "vol_slope";
    constexpr const char* transientAmt  = "transient_amount";
    constexpr const char* transientWave = "transient_wave";
    constexpr const char* snap          = "snap";
    constexpr const char* cutoff        = "cutoff";
    constexpr const char* reso          = "reso";
    constexpr const char* filterType    = "filter_type";
    constexpr const char* distortion    = "distortion";
    constexpr const char* volume        = "volume";
    constexpr const char* pan           = "pan";
}

using P = juce::AudioParameterFloat;
using PI = juce::AudioParameterInt;
using PC = juce::AudioParameterChoice;

juce::AudioProcessorValueTreeState::ParameterLayout
LxrDrumProcessor::buildParameterLayout()
{
    auto unit = juce::NormalisableRange<float> (0.0f, 1.0f);
    auto bipolar = juce::NormalisableRange<float> (-1.0f, 1.0f);

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Pitch params now map to a ±36 semitone offset from the played
    // MIDI note. 0.5 = play at the played note. Lower default for the
    // main osc so a played MIDI note 60 produces a kick-drum range
    // tone rather than a melodic pitch.
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pitch,         1), "Pitch",            unit,    0.30f));
    layout.add (std::make_unique<PC> (juce::ParameterID (pid::waveform,      1), "Waveform",
                                      juce::StringArray { "Sine", "Saw", "Triangle", "Rect", "Noise" }, 0));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::fmAmount,      1), "FM Amount",        unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::modPitch,      1), "Mod Pitch",        unit,    0.5f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pitchEgAmt,    1), "Pitch EG Amount",  bipolar, 0.5f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pitchDecay,    1), "Pitch Decay",      unit,    0.30f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volAttack,     1), "Volume Attack",    unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volDecay,      1), "Volume Decay",     unit,    0.50f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volSlope,      1), "Volume Slope",     unit,    0.50f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::transientAmt,  1), "Transient Amount", unit,    0.50f));
    layout.add (std::make_unique<PI> (juce::ParameterID (pid::transientWave, 1), "Transient Wave",   0, 7,    0));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::snap,          1), "Snap",             unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::cutoff,        1), "Cutoff",           unit,    1.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::reso,          1), "Resonance",        unit,    0.0f));
    layout.add (std::make_unique<PC> (juce::ParameterID (pid::filterType,    1), "Filter Type",
                                      juce::StringArray { "Off", "LP", "HP", "BP" }, 0));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::distortion,    1), "Distortion",       unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volume,        1), "Volume",           unit,    0.80f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pan,           1), "Pan",              unit,    0.50f));

    return layout;
}

LxrDrumProcessor::LxrDrumProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", buildParameterLayout())
{
    pPitch         = apvts.getRawParameterValue (pid::pitch);
    pWaveform      = apvts.getRawParameterValue (pid::waveform);
    pFmAmount      = apvts.getRawParameterValue (pid::fmAmount);
    pModPitch      = apvts.getRawParameterValue (pid::modPitch);
    pPitchEgAmt    = apvts.getRawParameterValue (pid::pitchEgAmt);
    pPitchDecay    = apvts.getRawParameterValue (pid::pitchDecay);
    pVolAttack     = apvts.getRawParameterValue (pid::volAttack);
    pVolDecay      = apvts.getRawParameterValue (pid::volDecay);
    pVolSlope      = apvts.getRawParameterValue (pid::volSlope);
    pTransientAmt  = apvts.getRawParameterValue (pid::transientAmt);
    pTransientWave = apvts.getRawParameterValue (pid::transientWave);
    pSnap          = apvts.getRawParameterValue (pid::snap);
    pCutoff        = apvts.getRawParameterValue (pid::cutoff);
    pReso          = apvts.getRawParameterValue (pid::reso);
    pFilterType    = apvts.getRawParameterValue (pid::filterType);
    pDistortion    = apvts.getRawParameterValue (pid::distortion);
    pVolume        = apvts.getRawParameterValue (pid::volume);
    pPan           = apvts.getRawParameterValue (pid::pan);
}

void LxrDrumProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    engine.prepare (sampleRate);
    pushParametersToEngine();
}

bool LxrDrumProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void LxrDrumProcessor::pushParametersToEngine() noexcept
{
    engine.setPitch         (pPitch        ->load());
    engine.setWaveform      ((int) pWaveform     ->load());
    engine.setFmAmount      (pFmAmount     ->load());
    engine.setModPitch      (pModPitch     ->load());
    engine.setPitchEgAmount (pPitchEgAmt   ->load());
    engine.setPitchDecay    (pPitchDecay   ->load());
    engine.setVolEgAttack   (pVolAttack    ->load());
    engine.setVolEgDecay    (pVolDecay     ->load());
    engine.setVolEgSlope    (pVolSlope     ->load());
    engine.setTransientAmt  (pTransientAmt ->load());
    engine.setTransientWave ((int) pTransientWave->load());
    engine.setSnap          (pSnap         ->load());
    engine.setFilterCutoff  (pCutoff       ->load());
    engine.setFilterReso    (pReso         ->load());
    engine.setFilterType    ((int) pFilterType   ->load());
    engine.setDistortion    (pDistortion   ->load());
    engine.setVolume        (pVolume       ->load());
    engine.setPan           (pPan          ->load());
}

void LxrDrumProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                     juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numCh      = buffer.getNumChannels();

    // Push current parameter values to the engine once per block. This
    // is fine because LXR parameters are coarse step controls (no fine
    // per-sample automation needed for drum synthesis). For sample-
    // accurate automation, walk the MIDI buffer in chunks instead.
    pushParametersToEngine();

    // Tempo-sync the LFO from host transport if available.
    if (auto* head = getPlayHead())
    {
        if (auto info = head->getPosition())
            if (auto bpm = info->getBpm())
                engine.setHostBpm (static_cast<float> (*bpm));
    }

    // Render mono into channel 0, then duplicate to channel 1 if stereo.
    float* mono = buffer.getWritePointer (0);

    int sampleCursor = 0;
    for (auto meta : midi)
    {
        const auto m = meta.getMessage();
        const int  pos = (int) meta.samplePosition;

        // Render up to the event timestamp first.
        if (pos > sampleCursor)
        {
            engine.process (mono + sampleCursor, pos - sampleCursor);
            sampleCursor = pos;
        }

        if (m.isNoteOn())
            engine.trigger (static_cast<uint8_t> (m.getVelocity()),
                            static_cast<uint8_t> (m.getNoteNumber()));
        // Note-offs intentionally ignored: LXR voices are one-shot;
        // the volume EG handles release.
    }

    if (sampleCursor < numSamples)
        engine.process (mono + sampleCursor, numSamples - sampleCursor);

    // Duplicate mono out to all channels.
    for (int ch = 1; ch < numCh; ++ch)
        buffer.copyFrom (ch, 0, buffer, 0, 0, numSamples);
}

juce::AudioProcessorEditor* LxrDrumProcessor::createEditor()
{
    return new LxrDrumEditor (*this);
}

void LxrDrumProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::MemoryOutputStream mos (dest, false);
    apvts.state.writeToStream (mos);
}

void LxrDrumProcessor::setStateInformation (const void* data, int size)
{
    auto t = juce::ValueTree::readFromData (data, (size_t) size);
    if (t.isValid())
        apvts.replaceState (t);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LxrDrumProcessor();
}
