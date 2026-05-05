/*
 * random.c
 *
 *  Originally created on: 07.04.2012 by Julian Schmidt (Sonic Potions LXR)
 *
 * --------------------------------------------------------------------
 * MODIFIED for desktop plugin build.
 *
 * The original implementation used the STM32F4 hardware RNG peripheral
 * (RCC_AHB2PeriphClockCmd, RNG_Cmd, RNG_GetRandomNumber). That has no
 * portable equivalent. We replace it here with an xorshift32 PRNG,
 * which is fast, deterministic-per-seed, and statistically adequate
 * for noise oscillators / dither / probability decisions.
 *
 * The xorshift32 algorithm is by George Marsaglia (public domain).
 *
 * Original copyright header preserved below in compliance with the
 * LXR redistribution terms (the file is part of the Sonic Potions LXR
 * drumsynth firmware; non-commercial use only).
 * --------------------------------------------------------------------
 *  Copyright 2013 Julian Schmidt
 *  Julian@sonic-potions.com
 *  See LXR_LICENSE.txt for the full original notice.
 * --------------------------------------------------------------------
 */

#include "random.h"
#include <stdint.h>
#include <time.h>

/* Per-process state. xorshift32 must never be seeded with zero, so the
 * default seed is the constant Marsaglia recommended. The plugin can
 * call lxr_random_seed() from its constructor to inject a host-specific
 * seed for reproducible output. */
static uint32_t s_rng_state = 0x9E3779B9u;

void initRng(void)
{
    /* Mix in something time-varying so two plugin instances loaded in
     * the same DAW session don't produce identical noise streams. */
    s_rng_state ^= (uint32_t)time(NULL);
    if (s_rng_state == 0u) s_rng_state = 0x9E3779B9u;
}

uint32_t GetRngValue(void)
{
    uint32_t x = s_rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    s_rng_state = x;
    return x;
}

/* Public hook for the plugin wrapper to set a deterministic seed. */
void lxr_random_seed(uint32_t seed)
{
    s_rng_state = seed ? seed : 0x9E3779B9u;
}
