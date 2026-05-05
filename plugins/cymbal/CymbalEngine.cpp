#include "CymbalEngine.h"
#include "../shared/LxrEngineBase.h"

extern "C" {
    #include "CymbalVoice.h"
    #include "SlopeEg2.h"
    #include "ResonantFilter.h"
    #include "distortion.h"
    #include "Oscillator.h"
}

#include <algorithm>
#include <cmath>

namespace lxr {

namespace {
    inline uint8_t toMidiData (float n) noexcept {
        n = std::clamp (n, 0.0f, 1.0f);
        return static_cast<uint8_t> (n * 127.0f + 0.5f);
    }
}

CymbalEngine::CymbalEngine()
{
    ensureGlobalInit();
    Cymbal_init();

    generator = [] (int16_t* buf, int size) {
        Cymbal_calcAsync();
        Cymbal_calcSyncBlock (buf, static_cast<uint8_t> (size));
    };

    accum.reset();
}

void CymbalEngine::prepare (double sr)         { currentSampleRate = sr; accum.reset(); }
void CymbalEngine::setHostBpm (float bpm) noexcept { lxr_setBpmFloat (bpm); }
void CymbalEngine::trigger (uint8_t v, uint8_t n) noexcept { Cymbal_trigger (v, n); }
void CymbalEngine::process (float* d, int n) noexcept { accum.fillBuffer (d, n, generator); }

// Carrier ----------------------------------------------------------------
//
// PITCH HANDLING NOTE: see HiHatEngine.cpp - we set midiFreq, not freq,
// so that the value survives the next trigger.

namespace {
    inline uint16_t pitchToMidiFreq (float norm01, int defaultNote = 63) noexcept
    {
        constexpr int range = 36;
        norm01 = std::clamp (norm01, 0.0f, 1.0f);
        int offset = static_cast<int> ((norm01 - 0.5f) * 2.0f * range);
        int note = std::clamp (defaultNote + offset, 0, 127);
        return static_cast<uint16_t> (note << 8);
    }
}

void CymbalEngine::setPitch (float n) noexcept
{
    cymbalVoice.osc.midiFreq = pitchToMidiFreq (n);
}

void CymbalEngine::setWaveform (int index) noexcept
{
    // Clamp to safe range - waveforms 5/6 (user samples) read from
    // SD-card memory we don't have on desktop and would crash.
    cymbalVoice.osc.waveform = static_cast<uint8_t> (std::clamp (index, 0, 4));
}

// FM modulators ----------------------------------------------------------

void CymbalEngine::setMod1Pitch (float n) noexcept
{
    cymbalVoice.modOsc.midiFreq = pitchToMidiFreq (n);
}

void CymbalEngine::setMod1Amount (float n) noexcept
{
    cymbalVoice.fmModAmount1 = std::clamp (n, 0.0f, 1.0f);
}

void CymbalEngine::setMod1Waveform (int index) noexcept
{
    // Same waveform set as the carrier, clamped to safe range.
    // Sine gives clean harmonic FM; Noise gives the noisy/metallic
    // character that makes a cymbal sound like a cymbal.
    cymbalVoice.modOsc.waveform = static_cast<uint8_t> (std::clamp (index, 0, 4));
}

void CymbalEngine::setMod2Pitch (float n) noexcept
{
    cymbalVoice.modOsc2.midiFreq = pitchToMidiFreq (n);
}

void CymbalEngine::setMod2Amount (float n) noexcept
{
    cymbalVoice.fmModAmount2 = std::clamp (n, 0.0f, 1.0f);
}

void CymbalEngine::setMod2Waveform (int index) noexcept
{
    cymbalVoice.modOsc2.waveform = static_cast<uint8_t> (std::clamp (index, 0, 4));
}

// Volume EG --------------------------------------------------------------

void CymbalEngine::setVolEgAttack (float n) noexcept
{
    slopeEg2_setAttack (&cymbalVoice.oscVolEg, toMidiData (n), 0);
}

void CymbalEngine::setVolEgDecay (float n) noexcept
{
    slopeEg2_setDecay (&cymbalVoice.oscVolEg, toMidiData (n), 0);
}

void CymbalEngine::setVolEgSlope (float n) noexcept
{
    slopeEg2_setSlope (&cymbalVoice.oscVolEg, toMidiData (n));
}

// Transient & snap -------------------------------------------------------

void CymbalEngine::setTransientAmt (float n) noexcept
{
    cymbalVoice.transGen.volume = std::clamp (n, 0.0f, 1.0f);
}

void CymbalEngine::setTransientWave (int index) noexcept
{
    cymbalVoice.transGen.waveform = static_cast<uint8_t> (std::clamp (index, 0, 7));
}

void CymbalEngine::setSnap (float n) noexcept
{
    cymbalVoice.snapEg.value = std::clamp (n, 0.0f, 1.0f) * 0.999f;
}

// Filter -----------------------------------------------------------------

void CymbalEngine::setFilterCutoff (float n) noexcept
{
    SVF_directSetFilterValue (&cymbalVoice.filter, cutoffSliderToSvfInput (n));
}

void CymbalEngine::setFilterReso (float n) noexcept
{
    SVF_setReso (&cymbalVoice.filter, std::clamp (n, 0.0f, 0.99f));
}

void CymbalEngine::setFilterType (int type) noexcept
{
    uint8_t t = 0;
    switch (type) {
        case 1: t = 0x1; break;
        case 2: t = 0x2; break;
        case 3: t = 0x4; break;
        default: t = 0; break;
    }
    cymbalVoice.filterType = t;
}

void CymbalEngine::setDistortion (float n) noexcept
{
    setDistortionShape (&cymbalVoice.distortion, toMidiData (n));
}

void CymbalEngine::setVolume (float n) noexcept
{
    cymbalVoice.vol = std::clamp (n, 0.0f, 1.0f);
}

void CymbalEngine::setPan (float n) noexcept
{
    Cymbal_setPan (toMidiData (n));
}

} // namespace lxr
