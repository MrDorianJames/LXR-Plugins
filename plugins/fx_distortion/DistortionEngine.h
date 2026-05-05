#pragma once

#include <cstdint>

extern "C" { struct DistStruct; typedef struct DistStruct Distortion; }

namespace lxr {

/**
 * DistortionEngine - the LXR per-voice distortion as a standalone effect.
 *
 * The DSP is a simple smooth waveshaper:
 *
 *   y = (1 + k) * x / (1 + k * |x|)
 *
 * where k = 2*(shape/128) / (1 - shape/128). At shape=0 it's nearly
 * linear (gentle saturation); as shape approaches 127 it gets very
 * aggressive. The shape is the only "amount" knob - there's no
 * separate drive vs. tone, the curve takes care of both. We add an
 * output-level trim because heavy shape values can really push the
 * volume up.
 */
class DistortionEngine
{
public:
    DistortionEngine();
    ~DistortionEngine();

    void prepare (double sampleRate);
    void reset() noexcept;

    void process (const float* src, float* dst, int numSamples) noexcept;

    void setShape  (float norm01) noexcept;   // 0..1 -> firmware 0..127
    void setOutput (float norm01) noexcept;   // 0..1, 0.5 = unity

private:
    struct State; State* state;
    float outputGain { 1.0f };
};

} // namespace lxr
