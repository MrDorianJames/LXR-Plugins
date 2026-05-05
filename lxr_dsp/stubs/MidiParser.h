/*
 * MidiParser.h - desktop stub
 *
 * Most of the firmware's MidiParser is host-specific (sysex over UART,
 * NRPN, sequencer interaction). The DSP layer needs only one helper:
 * midiParser_calcDetune, which converts a 0..127 detune value into a
 * pitch-multiplier near 1.0. We mirror the firmware implementation.
 */
#ifndef LXR_STUB_MIDI_PARSER_H
#define LXR_STUB_MIDI_PARSER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

float midiParser_calcDetune(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
