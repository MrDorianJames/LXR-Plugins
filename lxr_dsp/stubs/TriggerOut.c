#include "TriggerOut.h"

uint8_t trigger_dividerClockOut1     = 1;
uint8_t trigger_dividerClockOut2     = 1;
uint8_t trigger_prescalerClockInput  = PRE_4_PPQ;

uint8_t trigger_isGateModeOn(void) {
    /* Envelope mode: voices fade out using their amp EG.
     * Set to 1 if you want fixed-length gates instead. */
    return 0;
}
