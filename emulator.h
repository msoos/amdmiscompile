#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include <vector>
#include "commondata.h"
#include "convert_and_print.h"
#include "shareddata.h"

#include <iostream>
#include <iomanip>
#include <string.h>
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

class Emulator
{
public:
    void emulate(
        const uint64_t start
        , Output ret
    );
};

#endif //_EMULATOR_H_
