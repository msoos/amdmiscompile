__kernel __attribute__((reqd_work_group_size(LOCAL_SIZE, 1, 1)))
void fuzz (
    __global struct RunData *runData
    , __global uint *final
    , __global struct SanityCheck* sanity_check
) {
    unsigned state[STATE_SIZE];
    const unsigned k_global_offset = get_global_id(0) * NUM_OUTPUTS;

    for (unsigned i = 0; i < NUM_OUTPUTS; i++) {
        state[i] = 0;
    }

    //Put into bitslice
    for(uint i = 0; i < 32; i++) {
        ulong this_start = runData->start + get_global_id(0)*32 + i;

        for(uint i2 = 0; i2 < NUM_OUTPUTS; i2++) {
            state[i2] |= ((this_start >> i2) & 1UL) << i;
        }
    }

    fullclock(state);

    for (unsigned i = 0; i < NUM_OUTPUTS; i++) {
        final[get_global_id(0) * NUM_OUTPUTS + i] = state[i];
    }
    sanity_check->num_times_other_end = runData->num_times;
}
