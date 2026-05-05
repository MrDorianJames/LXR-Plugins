#pragma once

#include "../shared/BlockAccumulator.h"

#include <cstdint>

namespace lxr {

/**
 * SnareEngine - wraps the LXR snareVoice singleton (Snare.c).
 *
 * The snare is a noise + tonal-oscillator + bandpass-filter voice
 * with the classic "mix between osc and noise" parameter that makes
 * it equally good at clap and snare sounds.
 *
 * Like DrumEngine, this uses a global instance from the LXR firmware.
 * One snare per process.
 */
class SnareEngine
{
public:
    SnareEngine();
    ~SnareEngine() = default;

    void prepare (double sampleRate);
    void setHostBpm (float bpm) noexcept;
    void trigger (uint8_t velocity, uint8_t note) noexcept;
    void process (float* dst, int numSamples) noexcept;

    // Parameters
    void setOscPitch      (float norm01) noexcept;   // tonal osc pitch
    void setNoiseFilter   (float norm01) noexcept;   // noise filter cutoff
    void setMix           (float norm01) noexcept;   // 0=osc only, 1=noise only
    void setPitchEgAmount (float bipolar) noexcept;
    void setPitchDecay    (float norm01) noexcept;
    void setVolEgAttack   (float norm01) noexcept;
    void setVolEgDecay    (float norm01) noexcept;
    void setVolEgSlope    (float norm01) noexcept;
    void setTransientAmt  (float norm01) noexcept;
    void setTransientWave (int  index)   noexcept;
    void setSnap          (float norm01) noexcept;
    void setReso          (float norm01) noexcept;
    void setFilterType    (int  type)    noexcept;
    void setDistortion    (float norm01) noexcept;
    void setVolume        (float norm01) noexcept;
    void setPan           (float norm01) noexcept;

private:
    BlockAccumulator accum;
    BlockAccumulator::Generator generator;
    double currentSampleRate { 44100.0 };
};

} // namespace lxr
