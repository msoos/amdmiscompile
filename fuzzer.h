
#ifndef __FUZZER_H__
#define __FUZZER_H__

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "convert_and_print.h"
#include "commondata.h"
#include "openclhelper.h"
#include <cstring>
#include <vector>
#include <string>
using std::vector;
using std::string;

class SharedData;

class Fuzzer
{
public:
    Fuzzer(
        bool _optimize_compile
    );

    int fuzz();
    void set_verbosity(unsigned v);
    void set_device_num(const size_t _device_num);
    void set_shared_data(SharedData* sharedData);

private:
    //Main parameters
    SharedData* sharedData;
    void read_result();

    //Kernel building & error handling
    void set_kernel_args();

    //Run
    void calc_enque_update();
    int handle_returned_data();
    void check_sanity_func() const;
    void check_and_release_events();

    //Memory management
    void setup_graph_memory();

    //Releasing everything
    void free_graph_memory();
    void free_main_memory();
    void init_opencl();
    void free_datastructs();

    //Checking
    void backroll_bits(unsigned char *ret, const cl_uint *multiblock, const unsigned whichsub);

    //Graph memory
    cl_mem final_graph;
    cl_mem runData_graph;
    cl_mem sanity_check_graph;

    //Main memory
    RunData     runData;
    SanityCheck sanityCheck;
    cl_uint*  final_main;

    //Sizes
    size_t final_main_size;
    size_t local_item_size;

    //OpenCL system
    OpenCLHelper* ocl;
    bool optimize_compile;
};

#endif //__FUZZER_H__
