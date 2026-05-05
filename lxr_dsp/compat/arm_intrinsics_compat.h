/*
 * arm_intrinsics_compat.h - portable replacements for Cortex-M4 SIMD ops
 *
 * BufferTools and Snare use a small handful of ARM DSP intrinsics for
 * 16-bit saturating arithmetic. These instructions don't exist on
 * x86/x64, so we emulate them with portable C. Each operates on a
 * packed 32-bit word containing two int16 values (low-half, high-half).
 *
 * Performance is not critical here - the LXR runs at 44 kHz on a
 * 168 MHz Cortex-M4, so anything modern will outpace the original by
 * orders of magnitude even without SIMD.
 */
#ifndef LXR_ARM_INTRINSICS_COMPAT_H
#define LXR_ARM_INTRINSICS_COMPAT_H

#include <stdint.h>

#if defined(__ARM_ARCH_7EM__) || defined(__ARM_FEATURE_DSP)
/* Real Cortex-M4: use the toolchain's intrinsics. */
#  include "stm32f4xx.h"
#else

static inline int16_t lxr_sat16(int32_t x) {
    if (x >  32767) return  32767;
    if (x < -32768) return -32768;
    return (int16_t)x;
}

static inline uint32_t __QADD16(uint32_t a, uint32_t b) {
    int16_t a_lo = (int16_t)(a & 0xFFFF);
    int16_t a_hi = (int16_t)(a >> 16);
    int16_t b_lo = (int16_t)(b & 0xFFFF);
    int16_t b_hi = (int16_t)(b >> 16);
    uint16_t r_lo = (uint16_t)lxr_sat16((int32_t)a_lo + b_lo);
    uint16_t r_hi = (uint16_t)lxr_sat16((int32_t)a_hi + b_hi);
    return ((uint32_t)r_hi << 16) | r_lo;
}

static inline uint32_t __QSUB16(uint32_t a, uint32_t b) {
    int16_t a_lo = (int16_t)(a & 0xFFFF);
    int16_t a_hi = (int16_t)(a >> 16);
    int16_t b_lo = (int16_t)(b & 0xFFFF);
    int16_t b_hi = (int16_t)(b >> 16);
    uint16_t r_lo = (uint16_t)lxr_sat16((int32_t)a_lo - b_lo);
    uint16_t r_hi = (uint16_t)lxr_sat16((int32_t)a_hi - b_hi);
    return ((uint32_t)r_hi << 16) | r_lo;
}

/* __SSAT(x,n) saturates a signed int to n bits. The firmware uses it
 * mostly as __SSAT(value, 16). */
static inline int32_t __SSAT(int32_t val, uint32_t bits) {
    int32_t hi = (1 << (bits - 1)) - 1;
    int32_t lo = -(1 << (bits - 1));
    if (val > hi) return hi;
    if (val < lo) return lo;
    return val;
}

/* Count leading zeros - GCC builtin matches ARM CLZ semantics. */
static inline uint32_t __CLZ(uint32_t x) {
    return x == 0 ? 32u : (uint32_t)__builtin_clz(x);
}

#endif /* not Cortex-M4 */

#endif
