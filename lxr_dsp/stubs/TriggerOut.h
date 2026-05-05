/*
 * TriggerOut.h - desktop stub
 *
 * On the hardware the LXR has CV/Gate trigger outputs driven by GPIO.
 * On a software plugin there's no such concept. We provide the same
 * enums and function symbols so the voice code compiles unchanged, and
 * make the body of every function a no-op (or in the case of the gate
 * mode query, return 0 so voices use their internal envelopes).
 */
#ifndef LXR_STUB_TRIGGEROUT_H
#define LXR_STUB_TRIGGEROUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mirror the firmware's pin enum exactly so that expressions like
 * TRIGGER_1 + voiceNr keep their original meaning even though they're
 * never observed on a real GPIO. */
enum {
    TRIGGER_1 = 0,
    TRIGGER_2,
    TRIGGER_3,
    TRIGGER_4,
    TRIGGER_5,
    TRIGGER_6,
    TRIGGER_7,
    CLOCK_1,
    CLOCK_2,
    TRIGGER_RESET,
    TRIGGER_ALL,
    NUM_PINS,
};

enum Prescaler {
    PRE_1_PPQ  = 32/1,
    PRE_4_PPQ  = 32/4,
    PRE_8_PPQ  = 32/8,
    PRE_16_PPQ = 32/16,
    PRE_32_PPQ = 1,
};

typedef enum TriggerModes {
    TRIGGER_ON,
    TRIGGER_OFF,
    TRIGGER_PULSE
} triggerMode;

extern uint8_t trigger_dividerClockOut1;
extern uint8_t trigger_dividerClockOut2;
extern uint8_t trigger_prescalerClockInput;

uint8_t trigger_isGateModeOn(void);

/* Most of these are referenced by the voice and mixer code. None of
 * them do anything on a desktop build - the host owns the audio
 * pipeline, there's no eurorack output. */
static inline void trigger_init(void) {}
static inline void trigger_tick(void) {}
static inline void trigger_triggerVoice(uint8_t v, uint8_t mode) { (void)v; (void)mode; }
static inline void trigger_releaseVoice(uint8_t v) { (void)v; }
static inline void trigger_setPrescaler(uint8_t p) { (void)p; }

#ifdef __cplusplus
}
#endif

#endif
