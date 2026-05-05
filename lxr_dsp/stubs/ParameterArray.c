#include "ParameterArray.h"

uint8_t parameter_values[LXR_STUB_PARAM_COUNT] = {0};

/* All entries default-zero: ptr=NULL, type=TYPE_UINT8. Any DSP path
 * that tried to read through a NULL ptr would crash, but the only
 * caller (modulationNode.c) gates on `vm->destination == idx` and
 * destinations are never set in this build, so the table is dormant. */
const Parameter parameterArray[LXR_STUB_PARAM_COUNT] = {{0}};

void paramArray_setParameter(uint16_t idx, ptrValue val) {
    (void)idx; (void)val;
}
