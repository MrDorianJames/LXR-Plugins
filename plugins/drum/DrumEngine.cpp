#include "DrumEngine.h"
#include "../shared/LxrEngineBase.h"

extern "C" {
    #include "DrumVoice.h"
    #include "Decay.h"
    #include "SlopeEg2.h"
    #include "ResonantFilter.h"
    #include "distortion.h"
    #include "Oscillator.h"
}

#include <algorithm>
#include <cmath>

namespace lxr {

namespace {
    constexpr int kVoiceSlot = 0; // we use voiceArray[0]

    inline uint8_t toMidiData (float norm01) noexcept
    {
        norm01 = std::clamp (norm01, 0.0f, 1.0f);
        return static_cast<uint8_t> (norm01 * 127.0f + 0.5f);
    }
}

DrumEngine::DrumEngine()
{
    ensureGlobalInit();
    initDrumVoice();

    // Generator closure pulled by the BlockAccumulator. The LXR engine
    // expects a fixed 32-sample block each call; the accumulator never
    // asks for anything else, so the size param will always equal 32.
    generator = [] (int16_t* buf, int size)
    {
        calcDrumVoiceAsync (kVoiceSlot);
        calcDrumVoiceSyncBlock (kVoiceSlot, buf, static_cast<uint8_t> (size));
    };

    accum.reset();
}

void DrumEngine::prepare (double sr)
{
    currentSampleRate = sr;
    accum.reset();
    // The LXR engine expects 44.1k internally; we don't resample here.
    // See README "Known limitations" for a note on this.
}

void DrumEngine::setHostBpm (float bpm) noexcept
{
    lxr_setBpmFloat (bpm);
}

void DrumEngine::trigger (uint8_t velocity, uint8_t note) noexcept
{
    Drum_trigger (static_cast<uint8_t> (kVoiceSlot), velocity, note);
}

void DrumEngine::process (float* dst, int numSamples) noexcept
{
    accum.fillBuffer (dst, numSamples, generator);
}

// ---------- Oscillator section ----------
//
// PITCH HANDLING NOTE: see HiHatEngine.cpp for the full rationale.
// In short, osc_setBaseNote() called from Drum_trigger() overwrites
// osc->freq based on midiFreq + (note - SEQ_DEFAULT_NOTE), so we must
// write to midiFreq, not freq.

namespace {
    inline uint16_t pitchToMidiFreq (float norm01, int defaultNote = 63) noexcept
    {
        constexpr int range = 36;       // ±36 semitones
        norm01 = std::clamp (norm01, 0.0f, 1.0f);
        int offset = static_cast<int> ((norm01 - 0.5f) * 2.0f * range);
        int note = std::clamp (defaultNote + offset, 0, 127);
        return static_cast<uint16_t> (note << 8);
    }
}

void DrumEngine::setPitch (float norm01) noexcept
{
    // 0.5 = play at the played MIDI note; -36..+36 semitone offset.
    voiceArray[kVoiceSlot].osc.midiFreq = pitchToMidiFreq (norm01);
}

void DrumEngine::setWaveform (int index) noexcept
{
    // Valid waveforms in the LXR firmware: sine=0, saw=1, tri=2,
    // rec=3, noise=4. Indices 5 and 6 select user-sample slots, which
    // require the SD-card sample memory we stub out as zero on desktop;
    // selecting them crashes the oscillator code, so clamp to the safe
    // range here even though the parameter UI now restricts it too.
    voiceArray[kVoiceSlot].osc.waveform = static_cast<uint8_t> (std::clamp (index, 0, 4));
}

void DrumEngine::setFmAmount (float norm01) noexcept
{
    voiceArray[kVoiceSlot].fmModAmount = std::clamp (norm01, 0.0f, 1.0f);
}

void DrumEngine::setModPitch (float norm01) noexcept
{
    voiceArray[kVoiceSlot].modOsc.midiFreq = pitchToMidiFreq (norm01);
}

void DrumEngine::setPitchEgAmount (float bipolar) noexcept
{
    // pitch envelope amount is bipolar in the LXR (negative = downward sweep)
    voiceArray[kVoiceSlot].egPitchModAmount = std::clamp (bipolar, -1.0f, 1.0f);
}

void DrumEngine::setPitchDecay (float norm01) noexcept
{
    DecayEg_setDecay (&voiceArray[kVoiceSlot].oscPitchEg, toMidiData (norm01));
}

// ---------- Volume envelope ----------

void DrumEngine::setVolEgAttack (float norm01) noexcept
{
    slopeEg2_setAttack (&voiceArray[kVoiceSlot].oscVolEg, toMidiData (norm01), /*isSync=*/0);
}

void DrumEngine::setVolEgDecay (float norm01) noexcept
{
    slopeEg2_setDecay (&voiceArray[kVoiceSlot].oscVolEg, toMidiData (norm01), /*isSync=*/0);
}

void DrumEngine::setVolEgSlope (float norm01) noexcept
{
    slopeEg2_setSlope (&voiceArray[kVoiceSlot].oscVolEg, toMidiData (norm01));
}

// ---------- Transient generator ----------

void DrumEngine::setTransientAmt (float norm01) noexcept
{
    voiceArray[kVoiceSlot].transGen.volume = std::clamp (norm01, 0.0f, 1.0f);
}

void DrumEngine::setTransientWave (int index) noexcept
{
    voiceArray[kVoiceSlot].transGen.waveform = static_cast<uint8_t> (std::clamp (index, 0, 7));
}

void DrumEngine::setSnap (float norm01) noexcept
{
    // The snap EG is an exponential transient on top of the amp EG;
    // we just expose its peak level as a 0..1 scalar. The internal
    // attack/decay are fixed in the LXR.
    voiceArray[kVoiceSlot].snapEg.value = std::clamp (norm01, 0.0f, 1.0f) * 0.999f;
}

// ---------- Filter ----------

void DrumEngine::setFilterCutoff (float norm01) noexcept
{
    SVF_directSetFilterValue (&voiceArray[kVoiceSlot].filter,
                              cutoffSliderToSvfInput (norm01));
}

void DrumEngine::setFilterReso (float norm01) noexcept
{
    // SVF_setReso expects a feedback float; valid range is roughly 0..0.99.
    SVF_setReso (&voiceArray[kVoiceSlot].filter, std::clamp (norm01, 0.0f, 0.99f));
}

void DrumEngine::setFilterType (int type) noexcept
{
    // Bit layout per DrumVoice.h comment:
    //   bit 0 = LP, bit 1 = HP, bit 2 = BP
    // We expose 0=off, 1=LP, 2=HP, 3=BP for plugin sanity.
    uint8_t t = 0;
    switch (type) {
        case 1: t = 0x1; break;  // LP
        case 2: t = 0x2; break;  // HP
        case 3: t = 0x4; break;  // BP
        default: t = 0; break;   // bypass
    }
    voiceArray[kVoiceSlot].filterType = t;
}

// ---------- Distortion / Mix ----------

void DrumEngine::setDistortion (float norm01) noexcept
{
    setDistortionShape (&voiceArray[kVoiceSlot].distortion, toMidiData (norm01));
}

void DrumEngine::setVolume (float norm01) noexcept
{
    voiceArray[kVoiceSlot].vol = std::clamp (norm01, 0.0f, 1.0f);
}

void DrumEngine::setPan (float norm01) noexcept
{
    // setPan() in DrumVoice.c expects 0..127 with 64 being centre.
    // Use :: to call the C global, not recurse into ourselves.
    ::setPan (static_cast<uint8_t> (kVoiceSlot), toMidiData (norm01));
}

} // namespace lxr
