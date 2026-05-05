#include "SnareEngine.h"
#include "../shared/LxrEngineBase.h"

extern "C" {
    #include "Snare.h"
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
    inline uint8_t toMidiData (float n) noexcept {
        n = std::clamp (n, 0.0f, 1.0f);
        return static_cast<uint8_t> (n * 127.0f + 0.5f);
    }
}

SnareEngine::SnareEngine()
{
    ensureGlobalInit();
    Snare_init();

    generator = [] (int16_t* buf, int size) {
        Snare_calcAsync();
        Snare_calcSyncBlock (buf, static_cast<uint8_t> (size));
    };

    accum.reset();
}

void SnareEngine::prepare (double sr)         { currentSampleRate = sr; accum.reset(); }
void SnareEngine::setHostBpm (float bpm) noexcept { lxr_setBpmFloat (bpm); }
void SnareEngine::trigger (uint8_t v, uint8_t n) noexcept { Snare_trigger (v, n); }
void SnareEngine::process (float* d, int n) noexcept { accum.fillBuffer (d, n, generator); }

// Parameters --------------------------------------------------------------
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

void SnareEngine::setOscPitch (float n) noexcept
{
    snareVoice.osc.midiFreq = pitchToMidiFreq (n);
}

void SnareEngine::setNoiseFilter (float n) noexcept
{
    // The LXR snare voice is intentionally wired so that only the
    // noise oscillator passes through the filter; the tonal oscillator
    // is summed in unfiltered. This lets you shape the "snares" rattle
    // sizzle independently of the drum body's tonal hit. Filter type
    // (LP/HP/BP) and resonance are exposed separately.
    SVF_directSetFilterValue (&snareVoice.filter, cutoffSliderToSvfInput (n));
}

void SnareEngine::setMix (float n) noexcept
{
    snareVoice.mix = std::clamp (n, 0.0f, 1.0f);
}

void SnareEngine::setPitchEgAmount (float b) noexcept
{
    snareVoice.egPitchModAmount = std::clamp (b, -1.0f, 1.0f);
}

void SnareEngine::setPitchDecay (float n) noexcept
{
    DecayEg_setDecay (&snareVoice.oscPitchEg, toMidiData (n));
}

void SnareEngine::setVolEgAttack (float n) noexcept
{
    slopeEg2_setAttack (&snareVoice.oscVolEg, toMidiData (n), 0);
}

void SnareEngine::setVolEgDecay (float n) noexcept
{
    slopeEg2_setDecay (&snareVoice.oscVolEg, toMidiData (n), 0);
}

void SnareEngine::setVolEgSlope (float n) noexcept
{
    slopeEg2_setSlope (&snareVoice.oscVolEg, toMidiData (n));
}

void SnareEngine::setTransientAmt (float n) noexcept
{
    snareVoice.transGen.volume = std::clamp (n, 0.0f, 1.0f);
}

void SnareEngine::setTransientWave (int i) noexcept
{
    snareVoice.transGen.waveform = static_cast<uint8_t> (std::clamp (i, 0, 7));
}

void SnareEngine::setSnap (float n) noexcept
{
    snareVoice.snapEg.value = std::clamp (n, 0.0f, 1.0f) * 0.999f;
}

void SnareEngine::setReso (float n) noexcept
{
    SVF_setReso (&snareVoice.filter, std::clamp (n, 0.0f, 0.99f));
}

void SnareEngine::setFilterType (int type) noexcept
{
    uint8_t t = 0;
    switch (type) {
        case 1: t = 0x1; break;
        case 2: t = 0x2; break;
        case 3: t = 0x4; break;
        default: t = 0; break;
    }
    snareVoice.filterType = t;
}

void SnareEngine::setDistortion (float n) noexcept
{
    setDistortionShape (&snareVoice.distortion, toMidiData (n));
}

void SnareEngine::setVolume (float n) noexcept
{
    snareVoice.vol = std::clamp (n, 0.0f, 1.0f);
}

void SnareEngine::setPan (float n) noexcept
{
    Snare_setPan (toMidiData (n));
}

} // namespace lxr
