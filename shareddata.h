#ifndef SHARED_DATA
#define SHARED_DATA

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <iostream>
#include <iomanip>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <string>
#include <vector>
#include "commondata.h"
using std::cout;
using std::endl;
using std::string;

class SharedData
{
public:
    static void get_platform_and_devices(bool use_cpu);
    static cl_platform_id platform_id;
    static cl_device_id device_id[10];
    static cl_uint num_devices;
};


#endif //SHARED_DATA
