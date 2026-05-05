/*
 * sequencer.h - desktop stub
 *
 * The LFO module calls seq_getBpm() so its rate can sync to song tempo.
 * In a plugin we get tempo from the host and feed it back here. The
 * plugin sets the BPM each block from the host transport info.
 *
 * If you don't care about tempo-synced LFOs you can leave it at 120.
 */
#ifndef LXR_STUB_SEQUENCER_H
#define LXR_STUB_SEQUENCER_H

#include <stdint.h>

/* Default MIDI note used when a voice is triggered without a note number.
 * Mirrors firmware Sequencer/sequencer.h. */
#define SEQ_DEFAULT_NOTE 63

#ifdef __cplusplus
extern "C" {
#endif

float seq_getBpm(void);
void  seq_setBpm(uint16_t bpm);
void  lxr_setBpmFloat(float bpm);   /* convenience for hosts that report fractional bpm */

/* Other sequencer entry points the firmware may reference - all no-ops. */
static inline void seq_init(void) {}
static inline void seq_tick(void) {}
static inline void seq_sync(void) {}
static inline void seq_setRunning(uint8_t r) { (void)r; }

#ifdef __cplusplus
}
#endif

#endif
