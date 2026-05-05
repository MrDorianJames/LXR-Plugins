/*
 * ParameterArray.h - desktop stub
 *
 * In the firmware this declares a global parameter table written by the
 * MIDI/UI layer and read by the DSP. We bypass that pipeline and write
 * directly into the voice structs, so this stub provides the bare
 * minimum the DSP modules still reference: type tags, the parameter
 * table, and the ptrValue / Parameter types that modulationNode.h uses.
 */
#ifndef LXR_STUB_PARAMETER_ARRAY_H
#define LXR_STUB_PARAMETER_ARRAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Parameter value type tags - mirror MIDI/ParameterArray.h exactly so
 * modulationNode.c's switch statement still resolves. */
#define TYPE_UINT8              0
#define TYPE_FLT                1
#define TYPE_SPECIAL_F          2
#define TYPE_UINT32             3
#define TYPE_SPECIAL_P          4
#define TYPE_SPECIAL_FILTER_F   5

/* Sized loosely to match the firmware (~260 entries). 512 is plenty
 * and the storage is trivial. */
#define LXR_STUB_PARAM_COUNT 512
#define NUM_PARAMS LXR_STUB_PARAM_COUNT

extern uint8_t parameter_values[LXR_STUB_PARAM_COUNT];

/* ptrValue and Parameter come from MIDI/ParameterArray.h in the firmware
 * and are referenced by modulationNode.h. Mirror them exactly. */
typedef union ptrValueUnion {
    float    flt;
    uint32_t itg;
} ptrValue;

typedef struct ParameterStruct {
    void*   ptr;
    uint8_t type;
} Parameter;

/* In the firmware this is populated to point at every modulatable
 * parameter on every voice. modulationNode.c reads from it. We provide
 * an empty array (all entries .ptr=NULL, .type=TYPE_UINT8) - the modnode
 * code paths that actually dereference .ptr aren't reached unless the
 * plugin user wires up internal modulation, which is out of scope here.
 * For automation, use the host's parameter automation instead. */
extern const Parameter parameterArray[LXR_STUB_PARAM_COUNT];

/* Called by modulationNode.c to write modulated values back. We discard
 * - the host's automation system handles parameter changes instead. */
void paramArray_setParameter(uint16_t idx, ptrValue val);

#ifdef __cplusplus
}
#endif

#endif
