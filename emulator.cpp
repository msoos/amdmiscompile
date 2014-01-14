#include "emulator.h"
#include <assert.h>
#include <iostream>
#include <string.h>
#include "calc.h"
#include "calc.c"

void Emulator::emulate(
    const uint64_t start
    , Output ret_val
) {
    unsigned state[STATE_SIZE];
    unsigned long this_start = start;
    for(unsigned i = 0; i < NUM_OUTPUTS; i++) {
        state[i] = ((this_start >> i)&1) ? ~0U : 0U;
    }

    fullclock(state);

    memset(ret_val, 0, NUM_OUTPUTS_BYTES);
    for(size_t i = 0; i < NUM_OUTPUTS; i++) {
        ret_val[i/8] |= (state[i]&1) << (i % 8);
    }
}
