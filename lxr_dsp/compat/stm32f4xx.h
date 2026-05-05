/*
 * stm32f4xx.h - desktop compatibility shim
 *
 * The LXR DSP source includes this header in nearly every translation unit,
 * but it only relies on the integer typedefs (uint8_t, int16_t, etc.) and
 * a handful of feature macros. None of the actual STM32 peripheral access
 * is reachable from the DSP code path we use here.
 *
 * This file replaces the real ST header with a minimal shim that just
 * pulls in <stdint.h> and stubs out the section attributes used by the
 * firmware to place data in core-coupled memory.
 */
#ifndef LXR_COMPAT_STM32F4XX_H
#define LXR_COMPAT_STM32F4XX_H

#include <stdint.h>
#include <stddef.h>

/* The original config.h uses these section attributes to put data in
 * Cortex-M4 CCM RAM. On a desktop build they're meaningless - just elide. */
#ifdef INCCM
#  undef INCCM
#endif
#ifdef INCCMZ
#  undef INCCMZ
#endif
#define INCCM
#define INCCMZ

/* The firmware uses I2S_AudioFreq_44k as a magic constant; the desktop
 * sample rate comes from the host instead, but we still satisfy the
 * symbol so config.h compiles. */
#ifndef I2S_AudioFreq_44k
#  define I2S_AudioFreq_44k 44100
#endif

/* Some files reference DMA_MODE_ACTIVE - default it off. */
#ifndef DMA_MODE_ACTIVE
#  define DMA_MODE_ACTIVE 0
#endif

/* Provide portable replacements for the Cortex-M4 SIMD intrinsics
 * (__QADD16, __QSUB16, __SSAT) used by BufferTools and Snare. */
#include "arm_intrinsics_compat.h"

#endif /* LXR_COMPAT_STM32F4XX_H */
