#pragma once

#include "../shared/BlockAccumulator.h"

#include <memory>

#include <atomic>
#include <cstdint>

namespace lxr {

/**
 * DrumEngine
 *
 * Thin C++ wrapper around the LXR DrumVoice (slot 0 of voiceArray[]).
 * Responsibilities:
 *   1. provide a strongly-typed parameter API to the JUCE processor
 *   2. translate normalized float [0..1] parameter values into the
 *      varied scales and types the DSP code expects
 *   3. drive the BlockAccumulator with the voice's render function
 *
 * Multiple instances: the LXR DSP keeps the voice in a file-scope global
 * (voiceArray), which every instance in the same process would share.
 * To keep instances independent, each engine owns a private copy of that
 * state (see shared/VoiceContext.h) which is swapped into the global only
 * while this instance's methods run.
 */
class DrumEngine
{
public:
    DrumEngine();
    ~DrumEngine();

    /** Called by the plugin from prepareToPlay. */
    void prepare (double sampleRate);

    /** Optional: feed host tempo so any tempo-synced LFO tracks. */
    void setHostBpm (float bpm) noexcept;

    /** MIDI note-on. velocity is 0..127, note is the MIDI note. */
    void trigger (uint8_t velocity, uint8_t note) noexcept;

    /** Render numSamples float samples to dst. */
    void process (float* dst, int numSamples) noexcept;

    // --- Parameters (all inputs are normalized 0..1 unless stated) ---

    void setPitch         (float norm01) noexcept;   // base oscillator pitch
    void setWaveform      (int  index)   noexcept;   // 0..6 (sine/saw/tri/rec/noise/sample/...)
    void setFmAmount      (float norm01) noexcept;
    void setModPitch      (float norm01) noexcept;
    void setPitchEgAmount (float bipolar) noexcept;  // -1..+1
    void setPitchDecay    (float norm01) noexcept;

    void setVolEgAttack   (float norm01) noexcept;
    void setVolEgDecay    (float norm01) noexcept;
    void setVolEgSlope    (float norm01) noexcept;

    void setTransientAmt  (float norm01) noexcept;
    void setTransientWave (int  index)   noexcept;   // 0..N transient tables
    void setSnap          (float norm01) noexcept;

    void setFilterCutoff  (float norm01) noexcept;
    void setFilterReso    (float norm01) noexcept;
    void setFilterType    (int  type)    noexcept;   // 0=off,1=LP,2=HP,3=BP

    void setDistortion    (float norm01) noexcept;

    void setVolume        (float norm01) noexcept;   // 0..1 → 0..1 (used as float gain)
    void setPan           (float norm01) noexcept;   // 0..1 (0=L, 0.5=C, 1=R)

private:
    // Private copy of this instance's DSP voice state (see shared/VoiceContext.h).
    // Held through a pointer so this header needs no LXR C headers.
    struct Ctx;
    std::unique_ptr<Ctx> ctx;

    BlockAccumulator accum;
    BlockAccumulator::Generator generator; // bound once in ctor

    double currentSampleRate { 44100.0 };
};

} // namespace lxr
