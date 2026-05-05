#include "DistortionEngine.h"
#include "../shared/LxrEngineBase.h"

extern "C" {
    #include "distortion.h"
}

#include <algorithm>
#include <cmath>

namespace lxr {

struct DistortionEngine::State
{
    Distortion shaper {};
};

DistortionEngine::DistortionEngine()
    : state (new State)
{
    ensureGlobalInit();
    setDistortionShape (&state->shaper, 0);
}

DistortionEngine::~DistortionEngine()
{
    delete state;
}

void DistortionEngine::prepare (double /*sr*/) {}
void DistortionEngine::reset()      noexcept {}

void DistortionEngine::process (const float* src, float* dst, int numSamples) noexcept
{
    // distortion_calcSampleFloat is already per-sample float in/out -
    // no int16 staging needed. This is the cheapest plugin in the suite.
    for (int i = 0; i < numSamples; ++i)
    {
        float x = src[i];
        x = distortion_calcSampleFloat (&state->shaper, x);
        dst[i] = x * outputGain;
    }
}

void DistortionEngine::setShape (float norm01) noexcept
{
    norm01 = std::clamp (norm01, 0.0f, 1.0f);
    auto byte = static_cast<uint8_t> (norm01 * 127.0f + 0.5f);
    setDistortionShape (&state->shaper, byte);
}

void DistortionEngine::setOutput (float norm01) noexcept
{
    // Map 0..1 to 0..2 so 0.5 is unity and 1.0 doubles. Heavy shape
    // values can overdrive the output, so giving the user attenuation
    // headroom matters more than gain headroom.
    norm01 = std::clamp (norm01, 0.0f, 1.0f);
    outputGain = norm01 * 2.0f;
}

} // namespace lxr
