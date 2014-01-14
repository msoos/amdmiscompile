#ifndef __CONVERT_AND_PRINT_H__
#define __CONVERT_AND_PRINT_H__

#include <iostream>
#include <iomanip>
#include "assert.h"
#include <string>
#include <sstream>
using std::cout;
using std::endl;
using std::cerr;

inline std::string print_data(
    const unsigned char* data
    , int size
) {
    std::stringstream os;

    os << "0x";
    os << std::hex << std::setfill('0');
    for (int i = 0; i < size; i++) {
        os << std::setw(2) << (int)(data[i]);
    }
    os << std::setfill(' ') << std::dec;

    return os.str();
}

inline std::string print_bool(
    const bool* data
    , int size
) {
    std::stringstream os;

    //Print hexa
    os << "0b";
    for (int i = 0; i < size; i++) {
        os << std::setw(0) << (int)(data[i]);
    }

    return os.str();
}

#endif //__CONVERT_AND_PRINT_H__
