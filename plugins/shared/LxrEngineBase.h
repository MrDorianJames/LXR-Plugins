#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

// Forward-declare the C functions we need at engine-base level so we
// don't pull every voice header into every translation unit.
extern "C" {
    void initRng(void);
    void lxr_random_seed(uint32_t seed);
    void lxr_setBpmFloat(float bpm);
}

namespace lxr {

/**
 * Call once per process. Idempotent; subsequent calls are no-ops.
 * Initializes the RNG and any global state shared across voices.
 */
inline void ensureGlobalInit() noexcept
{
    static bool inited = false;
    if (!inited)
    {
        initRng();
        inited = true;
    }
}

/**
 * Maps a linear 0..1 slider value to the input expected by
 * SVF_directSetFilterValue() so that the knob feels musical.
 *
 * Why this exists: the firmware function does `filter->f = val * 0.45`,
 * which is a linear mapping into 0..0.45 of the normalised cutoff
 * frequency. At 44.1 kHz that's 0..19.85 kHz, but spread linearly. The
 * problem is that human hearing is logarithmic: each octave doubles
 * the frequency, and the audible range spans ~10 octaves. With a
 * linear knob, the bottom half (0..0.5) sweeps 0..9.9 kHz - that's
 * 9 octaves - and the top half (0.5..1.0) sweeps just 1 octave.
 * Result: the filter sounds "fully open" anywhere above ~0.7 on the
 * knob because there's no musical content up that high to filter.
 *
 * This applies a logarithmic curve so the slider sweeps ~10 octaves
 * (20 Hz to 20 kHz) evenly. Equal slider deltas produce equal
 * pitch deltas, like an analog filter knob.
 *
 *   slider 0.0 -> 20 Hz
 *   slider 0.25 -> ~112 Hz
 *   slider 0.5 -> ~632 Hz
 *   slider 0.75 -> ~3.5 kHz
 *   slider 1.0 -> 20 kHz (fully open)
 */
inline float cutoffSliderToSvfInput (float norm01) noexcept
{
    norm01 = std::clamp (norm01, 0.0f, 1.0f);

    // f_normalised = (20 / 44100) * 1000^position. The SVF function
    // multiplies by 0.45 internally, so we divide by 0.45 here so
    // slider=1.0 lands exactly at the firmware cap.
    constexpr float lowF  = 20.0f / 44100.0f;
    constexpr float decades = 1000.0f;            // 20 Hz -> 20 kHz = 3 decades
    constexpr float invSvfScale = 1.0f / 0.45f;

    const float f = lowF * std::pow (decades, norm01);
    return std::clamp (f * invSvfScale, 0.0f, 1.0f);
}

} // namespace lxr
