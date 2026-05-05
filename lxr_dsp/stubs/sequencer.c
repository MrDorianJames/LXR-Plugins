#include "sequencer.h"

static float s_bpm = 120.0f;

float seq_getBpm(void)         { return s_bpm; }
void  seq_setBpm(uint16_t bpm) { s_bpm = (float)bpm; }
void  lxr_setBpmFloat(float bpm) {
    if (bpm > 1.0f && bpm < 999.0f) s_bpm = bpm;
}
