/*
 * MidiVoiceControl.h - desktop stub
 *
 * The voice's calcAsync routines call voiceControl_noteOff via implicit
 * declaration (the firmware's calling translation unit doesn't actually
 * include this header in all build configs). To satisfy the linker we
 * declare it extern here and provide a no-op definition in the matching
 * .c file.
 */
#ifndef LXR_STUB_MIDI_VOICE_CONTROL_H
#define LXR_STUB_MIDI_VOICE_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void voiceControl_noteOff(uint8_t voice);
void voiceControl_noteOn (uint8_t voice, uint8_t note, uint8_t vel);

#ifdef __cplusplus
}
#endif

#endif
