#include "SampleMemory.h"
#include <string.h>

/* User-sample backing store: zero-filled, so any oscillator that tries
 * to play a user sample emits silence rather than reading garbage. */
const uint8_t lxr_user_sample_rom[SAMPLE_ROM_SIZE] = {0};

void sampleMemory_init(void) {}
void sampleMemory_loadSamples(void) {}

SampleInfo sampleMemory_getSampleInfo(uint8_t index) {
    (void)index;
    SampleInfo info;
    memset(&info, 0, sizeof(info));
    return info;
}

uint8_t sampleMemory_getNumSamples(void) { return 0; }
void    sampleMemory_setNumSamples(uint8_t num) { (void)num; }
