#pragma once

#include "../shared/BlockAccumulator.h"

#include <array>
#include <cstdint>

extern "C" {
    struct ResoFilterStruct;
    typedef struct ResoFilterStruct ResonantFilter;
}

namespace lxr {

/**
 * FilterEngine - the LXR's state-variable filter as a stereo effect.
 *
 * Wraps the same SVF_calcBlockZDF routine each voice uses internally.
 * The firmware filter expects int16 samples in 32-sample blocks; we
 * convert the host's float buffer in/out per channel. Two filter
 * instances are kept (one per channel) so stereo material stays
 * properly stereo - the SVF is stateful and you don't want to re-use
 * one instance for both channels.
 */
class FilterEngine
{
public:
    FilterEngine();
    ~FilterEngine();

    void prepare (double sampleRate, int maxBlockSize);
    void reset() noexcept;

    /** Process numSamples float samples from src to dst, in-place safe. */
    void process (const float* src, float* dst, int numSamples, int channel) noexcept;

    // Parameters - all 0..1 normalised unless noted.
    void setCutoff     (float norm01) noexcept;
    void setResonance  (float norm01) noexcept;
    void setDrive      (float norm01) noexcept;
    void setFilterType (int  type)    noexcept;   // 1=LP, 2=HP, 3=BP, 4=Unity BP, 5=Notch, 6=Peak, 7=Naive 2-pole

private:
    static constexpr int kBlock = 32;

    // Per-channel filter state (left, right). The LXR firmware lays
    // ResonantFilter out by value - we store opaque storage of the
    // right size and cast in the .cpp where the full type is known.
    // Allocated dynamically so we don't leak the C struct definition
    // into our header.
    struct State; State* state;

    int  currentType { 1 };
    float pendingCutoff { 1.0f }, pendingReso { 0.0f }, pendingDrive { 0.0f };
};

} // namespace lxr
