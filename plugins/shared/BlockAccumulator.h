#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace lxr {

/*
 * BlockAccumulator
 *
 * The LXR voices generate audio in fixed-size 32-sample chunks of int16.
 * Plugin hosts hand us float buffers of arbitrary size. This class
 * decouples the two: caller asks for N float samples; we run the LXR
 * generator (a callable that fills a 32-sample int16 buffer) however
 * many times needed and emit float samples one-by-one, holding any
 * leftover samples between host blocks.
 *
 * The generator callback is invoked with two int16_t* (we pass the
 * same pointer twice; if you ever need stereo you can split into two
 * buffers). It must fill exactly LXR_BLOCK samples.
 */
class BlockAccumulator
{
public:
    static constexpr int LXR_BLOCK = 32;

    using Generator = std::function<void(int16_t* buf, int size)>;

    BlockAccumulator() = default;

    void reset() noexcept
    {
        readIndex = LXR_BLOCK;     // empty: forces refill on first pull
        for (auto& s : staging) s = 0;
    }

    // Pull one float sample. Calls generator() lazily when staging is empty.
    inline float pullSample (Generator& gen) noexcept
    {
        if (readIndex >= LXR_BLOCK)
        {
            gen (staging.data(), LXR_BLOCK);
            readIndex = 0;
        }
        // int16 [-32768, 32767] -> float [-1.0, ~1.0)
        return static_cast<float> (staging[readIndex++]) * (1.0f / 32768.0f);
    }

    // Convenience: fill an entire float buffer.
    inline void fillBuffer (float* dst, int numSamples, Generator& gen) noexcept
    {
        for (int i = 0; i < numSamples; ++i)
            dst[i] = pullSample (gen);
    }

private:
    std::array<int16_t, LXR_BLOCK> staging {};
    int readIndex { LXR_BLOCK };
};

} // namespace lxr
