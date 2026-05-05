#include "MidiParser.h"

#define SEMITONE_UP 1.0594630943592952645618252949463f

/* Mirrors MIDI/MidiParser.c::midiParser_calcDetune:
 *   linear interpolation between 1.0 and one semitone up/down based
 *   on a 0..127 input. Note: the original firmware has a vestigial
 *   if/else where both branches do the same thing - preserved here
 *   to keep numerical behavior identical to the LXR. */
float midiParser_calcDetune(uint8_t value) {
    float frac = ((float)value / 127.f) - 0.5f;
    float cent = 1.f;
    if (cent >= 0) {
        cent += frac * (SEMITONE_UP - 1.f);
    } else {
        cent += frac * (SEMITONE_UP - 1.f);
    }
    return cent;
}
