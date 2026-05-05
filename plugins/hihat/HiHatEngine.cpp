#include "HiHatEngine.h"
#include "../shared/LxrEngineBase.h"

extern "C" {
    #include "HiHat.h"
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

HiHatEngine::HiHatEngine()
{
    ensureGlobalInit();
    HiHat_init();

    generator = [] (int16_t* buf, int size) {
        HiHat_calcAsync();
        HiHat_calcSyncBlock (buf, static_cast<uint8_t> (size));
    };

    accum.reset();
}

void HiHatEngine::prepare (double sr)         { currentSampleRate = sr; accum.reset(); }
void HiHatEngine::setHostBpm (float bpm) noexcept { lxr_setBpmFloat (bpm); }
void HiHatEngine::process (float* d, int n) noexcept { accum.fillBuffer (d, n, generator); }

void HiHatEngine::trigger (uint8_t velocity, uint8_t note) noexcept
{
    // Decide closed vs open based on mode.
    uint8_t isOpen = 0;
    switch (mode)
    {
        case Mode::AlwaysClosed: isOpen = 0; break;
        case Mode::AlwaysOpen:   isOpen = 1; break;
        case Mode::NoteSplit:
        default:
            isOpen = (note >= openSplitNote) ? 1 : 0;
            break;
    }
    HiHat_trigger (velocity, isOpen, note);
}

// Mode controls ----------------------------------------------------------

void HiHatEngine::setMode (int m) noexcept
{
    mode = static_cast<Mode> (std::clamp (m, 0, 2));
}

void HiHatEngine::setOpenSplitNote (int midiNote) noexcept
{
    openSplitNote = static_cast<uint8_t> (std::clamp (midiNote, 0, 127));
}

// Carrier & modulators ---------------------------------------------------
//
// PITCH HANDLING NOTE: the LXR sets oscillator pitch through the
// `midiFreq` field (upper byte = coarse semitones, lower byte = fine
// tune). On every trigger the firmware calls osc_setBaseNote() which
// reads midiFreq and rewrites osc->freq based on (midiFreq + played
// note - SEQ_DEFAULT_NOTE). Writing osc->freq directly is therefore
// pointless - it gets overwritten by the next trigger.
//
// Our pitch parameters map 0..1 to a coarse-semitone offset relative
// to SEQ_DEFAULT_NOTE (63). The mapping is:
//   0.0  -> SEQ_DEFAULT_NOTE - 36   (-3 octaves below played note)
//   0.5  -> SEQ_DEFAULT_NOTE        (plays at the played note)
//   1.0  -> SEQ_DEFAULT_NOTE + 36   (+3 octaves above played note)
// The hihat firmware default is +7 above (midiFreq = 70<<8), giving
// a slightly metallic ring; that's about norm01 = 0.6.

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

void HiHatEngine::setPitch (float n) noexcept
{
    hatVoice.osc.midiFreq = pitchToMidiFreq (n);
}

void HiHatEngine::setWaveform (int index) noexcept
{
    // Clamp to safe range - waveforms 5/6 (user samples) read from
    // SD-card memory we don't have on desktop and would crash.
    hatVoice.osc.waveform = static_cast<uint8_t> (std::clamp (index, 0, 4));
}

void HiHatEngine::setMod1Pitch (float n) noexcept
{
    hatVoice.modOsc.midiFreq = pitchToMidiFreq (n);
}

void HiHatEngine::setMod1Amount (float n) noexcept
{
    hatVoice.fmModAmount1 = std::clamp (n, 0.0f, 1.0f);
}

void HiHatEngine::setMod1Waveform (int index) noexcept
{
    // Sine = clean harmonic FM; Noise = grit/noise. The firmware
    // hardcodes Sine here at init - changing it gives you sounds the
    // hardware doesn't make out of the box.
    hatVoice.modOsc.waveform = static_cast<uint8_t> (std::clamp (index, 0, 4));
}

void HiHatEngine::setMod2Pitch (float n) noexcept
{
    hatVoice.modOsc2.midiFreq = pitchToMidiFreq (n);
}

void HiHatEngine::setMod2Amount (float n) noexcept
{
    hatVoice.fmModAmount2 = std::clamp (n, 0.0f, 1.0f);
}

void HiHatEngine::setMod2Waveform (int index) noexcept
{
    // Firmware default = Noise. This is what makes the hihat sound
    // hi-hat-like rather than bell-like. Try Sine + heavy mod2 amount
    // for clean inharmonic FM tones (gongs, bells, clangs).
    hatVoice.modOsc2.waveform = static_cast<uint8_t> (std::clamp (index, 0, 4));
}

// Closed / open decays.
//
// The firmware uses these as the per-block subtraction amount applied
// to the volume envelope value (which starts at 1.0 each trigger and
// counts down to 0). Writing the user's 0..1 slider value directly
// would be totally wrong:
//   - 0.0  -> held forever (no decrement)
//   - 0.05 -> already audibly instant
//   - 0.5+ -> snaps to silence in <1 block
// Almost the whole musical range lives in 0..0.005.
//
// The firmware's slopeEg2_calcDecay() applies an exponential curve
// (k = TIME_K_DECAY ~= 1998) that maps a 0..127 byte to a useful
// per-block coefficient. Slider 0.0 -> instant decay, slider 1.0 ->
// effectively held. We route through that helper so the UX matches
// the LXR-02 hardware's decay-time dial.
void HiHatEngine::setDecayClosed (float n) noexcept
{
    hatVoice.decayClosed = slopeEg2_calcDecay (toMidiData (n));
}

void HiHatEngine::setDecayOpen (float n) noexcept
{
    hatVoice.decayOpen = slopeEg2_calcDecay (toMidiData (n));
}

// Volume EG attack/slope (the decay portion is overridden by the
// closed/open decay parameters above).
void HiHatEngine::setVolEgAttack (float n) noexcept
{
    slopeEg2_setAttack (&hatVoice.oscVolEg, toMidiData (n), 0);
}

void HiHatEngine::setVolEgSlope (float n) noexcept
{
    slopeEg2_setSlope (&hatVoice.oscVolEg, toMidiData (n));
}

// Transient & snap -------------------------------------------------------

void HiHatEngine::setTransientAmt (float n) noexcept
{
    hatVoice.transGen.volume = std::clamp (n, 0.0f, 1.0f);
}

void HiHatEngine::setTransientWave (int index) noexcept
{
    hatVoice.transGen.waveform = static_cast<uint8_t> (std::clamp (index, 0, 7));
}

void HiHatEngine::setSnap (float n) noexcept
{
    hatVoice.snapEg.value = std::clamp (n, 0.0f, 1.0f) * 0.999f;
}

// Filter -----------------------------------------------------------------

void HiHatEngine::setFilterCutoff (float n) noexcept
{
    SVF_directSetFilterValue (&hatVoice.filter, cutoffSliderToSvfInput (n));
}

void HiHatEngine::setFilterReso (float n) noexcept
{
    SVF_setReso (&hatVoice.filter, std::clamp (n, 0.0f, 0.99f));
}

void HiHatEngine::setFilterType (int type) noexcept
{
    uint8_t t = 0;
    switch (type) {
        case 1: t = 0x1; break;
        case 2: t = 0x2; break;
        case 3: t = 0x4; break;
        default: t = 0; break;
    }
    hatVoice.filterType = t;
}

void HiHatEngine::setDistortion (float n) noexcept
{
    setDistortionShape (&hatVoice.distortion, toMidiData (n));
}

void HiHatEngine::setVolume (float n) noexcept
{
    hatVoice.vol = std::clamp (n, 0.0f, 1.0f);
}

void HiHatEngine::setPan (float n) noexcept
{
    HiHat_setPan (toMidiData (n));
}

} // namespace lxr
