#pragma once

#include "../shared/BlockAccumulator.h"
#include <cstdint>

namespace lxr {

/**
 * HiHatEngine - wraps the LXR hatVoice singleton (HiHat.c).
 *
 * Architecturally the hat is a 3-op FM voice like the cymbal, but with
 * two separate decay-time settings - one for closed hat triggers, one
 * for open hat. The triggering layer chooses which to use based on the
 * incoming MIDI note (see openSplitNote below) or a forced mode.
 */
class HiHatEngine
{
public:
    enum class Mode { NoteSplit = 0, AlwaysClosed = 1, AlwaysOpen = 2 };

    HiHatEngine();
    ~HiHatEngine() = default;

    void prepare (double sampleRate);
    void setHostBpm (float bpm) noexcept;
    void process (float* dst, int numSamples) noexcept;

    /** MIDI note-on. The note number is also used to decide closed vs
     *  open when in NoteSplit mode. */
    void trigger (uint8_t velocity, uint8_t note) noexcept;

    // Mode controls
    void setMode          (int mode)     noexcept;     // 0/1/2 -> Mode enum
    void setOpenSplitNote (int midiNote) noexcept;     // 0..127

    // Carrier & FM
    void setPitch         (float norm01) noexcept;
    void setWaveform      (int   index)  noexcept;
    void setMod1Pitch     (float norm01) noexcept;
    void setMod1Amount    (float norm01) noexcept;
    void setMod1Waveform  (int   index)  noexcept;
    void setMod2Pitch     (float norm01) noexcept;
    void setMod2Amount    (float norm01) noexcept;
    void setMod2Waveform  (int   index)  noexcept;

    // Two decays - closed and open
    void setDecayClosed   (float norm01) noexcept;
    void setDecayOpen     (float norm01) noexcept;

    // Volume EG (shared with closed/open path - separate from decayClosed/Open)
    void setVolEgAttack   (float norm01) noexcept;
    void setVolEgSlope    (float norm01) noexcept;

    void setTransientAmt  (float norm01) noexcept;
    void setTransientWave (int   index)  noexcept;
    void setSnap          (float norm01) noexcept;

    void setFilterCutoff  (float norm01) noexcept;
    void setFilterReso    (float norm01) noexcept;
    void setFilterType    (int   type)   noexcept;

    void setDistortion    (float norm01) noexcept;
    void setVolume        (float norm01) noexcept;
    void setPan           (float norm01) noexcept;

private:
    BlockAccumulator accum;
    BlockAccumulator::Generator generator;
    double currentSampleRate { 44100.0 };

    Mode    mode           { Mode::NoteSplit };
    uint8_t openSplitNote  { 44 };  // GM: closed hat=42, open hat=46
};

} // namespace lxr
