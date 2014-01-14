#ifndef __OPENCL_HELPER_H__
#define __OPENCL_HELPER_H__

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#include <string>
#include <vector>
#include <limits>

#include "commondata.h"

using std::vector;
using std::string;

class OpenCLHelper
{
public:
    OpenCLHelper(
        unsigned _verbosity
        , unsigned _device_num
        , bool _enable_opt = true
    ) :
        verbosity(_verbosity)
        , device_num(_device_num)
        , enable_opt(_enable_opt)
    {
    }


    //Setup
    void setup_device_context();
    void setup_background(
        const vector<string>& filenames
        , string kernelname
    );
    void release_all();

    //Errors
    void decode_enqueue_readwrite_error(const cl_int err);
    void print_enqueue_problem(cl_int ret);
    void handle_kernel_arg_error(cl_int ret);

    //Common vars
    cl_kernel kernel;
    cl_context context;
    cl_program program;
    cl_command_queue command_queue;

private:
    //Building
    vector<string> sources;
    vector<size_t> source_sizes;
    void read_sources(const vector<string>& filenames);
    void get_source_sizes();
    string read_source(const string filename);
    void print_build_log();
    void build_program();

    void get_device_and_kernel_data(
        size_t workgroup_multiplier = std::numeric_limits<size_t>::max()
    );

    //Setup
    unsigned verbosity;
    unsigned device_num;
    bool enable_opt;
};

#endif //__OPENCL_HELPER_H__