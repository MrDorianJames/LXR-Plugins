/*
 * SampleMemory.h - desktop stub
 *
 * On the hardware this manages user samples loaded from SD card into
 * STM32 flash. We disable user samples entirely in the plugin build;
 * factory ROM waveforms (sine/saw/tri/rec wavetables and the embedded
 * crash sample) still work because they're const tables compiled in.
 */
#ifndef LXR_STUB_SAMPLEMEMORY_H
#define LXR_STUB_SAMPLEMEMORY_H

#include <stdint.h>

#define SAMPLE_ROM_SIZE  ((uint32_t)0x00078E70)

typedef struct SampleInfoStruct {
    char     name[3];
    uint16_t size;
    uint32_t offset;
} SampleInfo;

#ifdef __cplusplus
extern "C" {
#endif

void       sampleMemory_init(void);
void       sampleMemory_loadSamples(void);
SampleInfo sampleMemory_getSampleInfo(uint8_t index);
uint8_t    sampleMemory_getNumSamples(void);
void       sampleMemory_setNumSamples(uint8_t num);

/* In the firmware Oscillator.c reads from a flash-mapped block at
 * SAMPLE_INFO_START_ADDRESS. We provide a zeroed buffer so any oscillator
 * code path that picks "user sample" oscillator type just produces silence
 * instead of dereferencing garbage. */
extern const uint8_t lxr_user_sample_rom[SAMPLE_ROM_SIZE];

#ifdef __cplusplus
}
#endif

#endif
