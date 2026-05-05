#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace pid {
    constexpr const char* pitch         = "pitch";
    constexpr const char* waveform      = "waveform";
    constexpr const char* mod1Pitch     = "mod1_pitch";
    constexpr const char* mod1Amount    = "mod1_amount";
    constexpr const char* mod1Waveform  = "mod1_waveform";
    constexpr const char* mod2Pitch     = "mod2_pitch";
    constexpr const char* mod2Amount    = "mod2_amount";
    constexpr const char* mod2Waveform  = "mod2_waveform";
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

using P  = juce::AudioParameterFloat;
using PI = juce::AudioParameterInt;
using PC = juce::AudioParameterChoice;

juce::AudioProcessorValueTreeState::ParameterLayout
LxrCymbalProcessor::buildParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto unit = juce::NormalisableRange<float> (0.0f, 1.0f);

    // Defaults chosen for a recognisable bell/cymbal sound out of the box:
    // mid-pitched carrier, two modulators set to harmonically rich ratios,
    // moderate FM depth, snappy attack with long decay.
    // Pitch defaults are slight upward offsets - cymbals want some
    // brightness above the played note. ±36 semitones range now.
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pitch,         1), "Pitch",            unit,    0.55f));
    layout.add (std::make_unique<PC> (juce::ParameterID (pid::waveform,      1), "Waveform",
                                      juce::StringArray { "Sine", "Saw", "Triangle", "Rect", "Noise" }, 0));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::mod1Pitch,     1), "FM1",              unit,    0.60f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::mod1Amount,    1), "Gain1",            unit,    0.55f));
    // Mod 1 default = Sine (firmware init's modOsc.waveform = SINE).
    layout.add (std::make_unique<PC> (juce::ParameterID (pid::mod1Waveform,  1), "Waveform 1",
                                      juce::StringArray { "Sine", "Saw", "Triangle", "Rect", "Noise" }, 0));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::mod2Pitch,     1), "FM2",              unit,    0.65f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::mod2Amount,    1), "Gain2",            unit,    0.45f));
    // Mod 2 default = Noise - this is what gives cymbals their inharmonic,
    // metallic character. Switch to Sine for cleaner bell tones.
    layout.add (std::make_unique<PC> (juce::ParameterID (pid::mod2Waveform,  1), "Waveform 2",
                                      juce::StringArray { "Sine", "Saw", "Triangle", "Rect", "Noise" }, 4));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volAttack,     1), "Volume Attack",    unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volDecay,      1), "Volume Decay",     unit,    0.75f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volSlope,      1), "Volume Slope",     unit,    0.55f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::transientAmt,  1), "Transient Amount", unit,    0.30f));
    layout.add (std::make_unique<PI> (juce::ParameterID (pid::transientWave, 1), "Transient Wave",   0, 7,    0));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::snap,          1), "Snap",             unit,    0.10f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::cutoff,        1), "Cutoff",           unit,    1.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::reso,          1), "Resonance",        unit,    0.0f));
    layout.add (std::make_unique<PC> (juce::ParameterID (pid::filterType,    1), "Filter Type",
                                      juce::StringArray { "Off", "LP", "HP", "BP" }, 2));   // default HP for cymbal sheen
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::distortion,    1), "Distortion",       unit,    0.0f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::volume,        1), "Volume",           unit,    0.75f));
    layout.add (std::make_unique<P>  (juce::ParameterID (pid::pan,           1), "Pan",              unit,    0.50f));

    return layout;
}

LxrCymbalProcessor::LxrCymbalProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", buildParameterLayout())
{
    pPitch         = apvts.getRawParameterValue (pid::pitch);
    pWaveform      = apvts.getRawParameterValue (pid::waveform);
    pMod1Pitch     = apvts.getRawParameterValue (pid::mod1Pitch);
    pMod1Amount    = apvts.getRawParameterValue (pid::mod1Amount);
    pMod1Waveform  = apvts.getRawParameterValue (pid::mod1Waveform);
    pMod2Pitch     = apvts.getRawParameterValue (pid::mod2Pitch);
    pMod2Amount    = apvts.getRawParameterValue (pid::mod2Amount);
    pMod2Waveform  = apvts.getRawParameterValue (pid::mod2Waveform);
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

void LxrCymbalProcessor::prepareToPlay (double sr, int)
{
    engine.prepare (sr);
    pushParametersToEngine();
}

bool LxrCymbalProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto out = l.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void LxrCymbalProcessor::pushParametersToEngine() noexcept
{
    engine.setPitch         (pPitch         ->load());
    engine.setWaveform      ((int) pWaveform      ->load());
    engine.setMod1Pitch     (pMod1Pitch     ->load());
    engine.setMod1Amount    (pMod1Amount    ->load());
    engine.setMod1Waveform  ((int) pMod1Waveform ->load());
    engine.setMod2Pitch     (pMod2Pitch     ->load());
    engine.setMod2Amount    (pMod2Amount    ->load());
    engine.setMod2Waveform  ((int) pMod2Waveform ->load());
    engine.setVolEgAttack   (pVolAttack     ->load());
    engine.setVolEgDecay    (pVolDecay      ->load());
    engine.setVolEgSlope    (pVolSlope      ->load());
    engine.setTransientAmt  (pTransientAmt  ->load());
    engine.setTransientWave ((int) pTransientWave ->load());
    engine.setSnap          (pSnap          ->load());
    engine.setFilterCutoff  (pCutoff        ->load());
    engine.setFilterReso    (pReso          ->load());
    engine.setFilterType    ((int) pFilterType    ->load());
    engine.setDistortion    (pDistortion    ->load());
    engine.setVolume        (pVolume        ->load());
    engine.setPan           (pPan           ->load());
}

void LxrCymbalProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
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

juce::AudioProcessorEditor* LxrCymbalProcessor::createEditor()
{
    return new LxrCymbalEditor (*this);
}

void LxrCymbalProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::MemoryOutputStream mos (dest, false);
    apvts.state.writeToStream (mos);
}

void LxrCymbalProcessor::setStateInformation (const void* data, int size)
{
    auto t = juce::ValueTree::readFromData (data, (size_t) size);
    if (t.isValid()) apvts.replaceState (t);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LxrCymbalProcessor();
}
