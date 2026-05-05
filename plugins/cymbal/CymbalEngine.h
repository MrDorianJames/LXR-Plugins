#pragma once

#include "../shared/BlockAccumulator.h"
#include <cstdint>

namespace lxr {

/**
 * CymbalEngine - wraps the LXR cymbalVoice singleton (CymbalVoice.c).
 *
 * The cymbal is a 3-operator FM voice: one carrier oscillator plus two
 * modulator oscillators, each with its own FM-into-carrier amount. This
 * is what gives it its metallic, inharmonic character that the simpler
 * drum/snare voices can't produce.
 */
class CymbalEngine
{
public:
    CymbalEngine();
    ~CymbalEngine() = default;

    void prepare (double sampleRate);
    void setHostBpm (float bpm) noexcept;
    void trigger (uint8_t velocity, uint8_t note) noexcept;
    void process (float* dst, int numSamples) noexcept;

    // Carrier
    void setPitch         (float norm01) noexcept;
    void setWaveform      (int   index)  noexcept;

    // First modulator
    void setMod1Pitch     (float norm01) noexcept;
    void setMod1Amount    (float norm01) noexcept;
    void setMod1Waveform  (int   index)  noexcept;

    // Second modulator
    void setMod2Pitch     (float norm01) noexcept;
    void setMod2Amount    (float norm01) noexcept;
    void setMod2Waveform  (int   index)  noexcept;

    // Volume envelope
    void setVolEgAttack   (float norm01) noexcept;
    void setVolEgDecay    (float norm01) noexcept;
    void setVolEgSlope    (float norm01) noexcept;

    // Misc
    void setTransientAmt  (float norm01) noexcept;
    void setTransientWave (int   index)  noexcept;
    void setSnap          (float norm01) noexcept;

    // Filter
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
};

} // namespace lxr
