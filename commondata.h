#ifndef _COMMON_DATA_H_
#define _COMMON_DATA_H_

#define LOCAL_SIZE 64
#define NUM_OUTPUTS 32
#define NUM_OUTPUTS_BYTES (4)
#define STATE_SIZE 60

typedef unsigned char Output[NUM_OUTPUTS_BYTES];

struct RunData
{
    unsigned long num_times;
    long unsigned int start;
};

struct SanityCheck
{
    unsigned int num_times_other_end;
};

#endif //_COMMON_DATA_H_
