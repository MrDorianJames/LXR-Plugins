#include "FilterEngine.h"
#include "../shared/LxrEngineBase.h"

extern "C" {
    #include "ResonantFilter.h"
}

#include <algorithm>
#include <array>
#include <cmath>

namespace lxr {

struct FilterEngine::State
{
    // One filter per channel - SVF is stateful, can't share.
    ResonantFilter chans[2];
    // Staging area for the int16 conversion; sized to one LXR block.
    std::array<int16_t, kBlock> staging {};
};

FilterEngine::FilterEngine()
    : state (new State)
{
    ensureGlobalInit();
    SVF_init (&state->chans[0]);
    SVF_init (&state->chans[1]);
    setCutoff    (1.0f);
    setResonance (0.0f);
    setDrive     (0.0f);
    setFilterType (1);  // LP default
}

FilterEngine::~FilterEngine()
{
    delete state;
}

void FilterEngine::prepare (double /*sr*/, int /*maxBlock*/)
{
    SVF_reset (&state->chans[0]);
    SVF_reset (&state->chans[1]);
}

void FilterEngine::reset() noexcept
{
    SVF_reset (&state->chans[0]);
    SVF_reset (&state->chans[1]);
}

void FilterEngine::process (const float* src, float* dst, int numSamples, int channel) noexcept
{
    if (channel < 0 || channel > 1) return;

    auto& filter  = state->chans[channel];
    auto& staging = state->staging;

    int remaining = numSamples;
    int srcOffset = 0;

    while (remaining > 0)
    {
        const int chunk = std::min (remaining, kBlock);

        // float [-1,+1) -> int16. Soft-clip lightly so loud input doesn't
        // wrap around through int16 overflow before reaching the filter's
        // own non-linearity.
        for (int i = 0; i < chunk; ++i)
        {
            float x = std::clamp (src[srcOffset + i], -1.0f, 1.0f);
            staging[i] = static_cast<int16_t> (x * 32767.0f);
        }
        // Pad with zeros if the host gave us less than a full block this
        // pass - SVF_calcBlockZDF doesn't bound-check internally.
        for (int i = chunk; i < kBlock; ++i)
            staging[i] = 0;

        SVF_calcBlockZDF (&filter,
                          static_cast<uint8_t> (currentType),
                          staging.data(),
                          static_cast<uint8_t> (kBlock));

        // int16 back to float.
        constexpr float invScale = 1.0f / 32767.0f;
        for (int i = 0; i < chunk; ++i)
            dst[srcOffset + i] = static_cast<float> (staging[i]) * invScale;

        srcOffset += chunk;
        remaining -= chunk;
    }
}

// ----- Parameters ----------------------------------------------------------

void FilterEngine::setCutoff (float norm01) noexcept
{
    pendingCutoff = std::clamp (norm01, 0.0f, 1.0f);
    const float svfInput = cutoffSliderToSvfInput (pendingCutoff);
    SVF_directSetFilterValue (&state->chans[0], svfInput);
    SVF_directSetFilterValue (&state->chans[1], svfInput);
}

void FilterEngine::setResonance (float norm01) noexcept
{
    // Cap at 0.99 to keep the SVF stable at extreme resonance.
    pendingReso = std::clamp (norm01, 0.0f, 0.99f);
    SVF_setReso (&state->chans[0], pendingReso);
    SVF_setReso (&state->chans[1], pendingReso);
}

void FilterEngine::setDrive (float norm01) noexcept
{
    pendingDrive = std::clamp (norm01, 0.0f, 1.0f);
    // SVF_setDrive expects a 0..127 byte. Lower bound to 1 so the
    // multiplier never collapses the signal to zero.
    auto byte = static_cast<uint8_t> (1 + pendingDrive * 126.0f + 0.5f);
    SVF_setDrive (&state->chans[0], byte);
    SVF_setDrive (&state->chans[1], byte);
}

void FilterEngine::setFilterType (int type) noexcept
{
    // Match the firmware enum: 1=LP, 2=HP, 3=BP, 4=Unity BP, 5=Notch,
    // 6=Peak, 7=Naive 2-pole. Out-of-range values fall back to LP.
    if (type < 1 || type > 7) type = 1;
    currentType = type;
}

} // namespace lxr
