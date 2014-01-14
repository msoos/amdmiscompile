#include <iostream>
#include "fuzzer.h"
#include "shareddata.h"

#include "assert.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <time.h>
#include <string.h>
#include <sstream>
#include "emulator.h"

using std::cout;
using std::cerr;
using std::endl;

Fuzzer::Fuzzer(
    bool _optimize_compile
) :
    sharedData(NULL)
    , optimize_compile(_optimize_compile)
{
    local_item_size = LOCAL_SIZE;
    final_main_size = NUM_OUTPUTS * local_item_size;

    cout << "Item size: " << local_item_size << endl;
}

void Fuzzer::backroll_bits(
    unsigned char *ret
    , const cl_uint *multiblock
    , const unsigned whichsub
) {
    memset(ret, 0, NUM_OUTPUTS_BYTES);
    for(size_t i = 0; i < NUM_OUTPUTS; i++) {
        unsigned char bit = (multiblock[i] >> whichsub) & 1;
        ret[i/8] |= bit << (i % 8);
    }
}

void Fuzzer::setup_graph_memory()
{
    int ret;

    runData_graph = clCreateBuffer(
        ocl->context
        , CL_MEM_READ_WRITE
        , sizeof(RunData)
        , NULL
        , &ret
    );
    if (ret != CL_SUCCESS) {
        cout << "Cannot set up runData buffer" << endl;
        exit(-1);
    }

    sanity_check_graph = clCreateBuffer(
        ocl->context
        , CL_MEM_WRITE_ONLY
        , sizeof(SanityCheck)
        , NULL
        , &ret
    );
    if (ret != CL_SUCCESS) {
        cout << "Cannot set up sanityCheck buffer" << endl;
        exit(-1);
    }

    final_graph = clCreateBuffer(
        ocl->context
        , CL_MEM_READ_WRITE
        , final_main_size * sizeof(cl_uint)
        , NULL
        , &ret
    );
    if (ret != CL_SUCCESS) {
        cout << "Cannot set up final output buffer" << endl;
        exit(-1);
    }

    cout << "Set up graph mem " << endl;
}

void Fuzzer::set_kernel_args()
{
    cl_int ret;
    cl_uint numArg = 0;

    //RunData
    ret = clSetKernelArg(ocl->kernel, numArg++, sizeof(cl_mem), &runData_graph);
    if (ret != CL_SUCCESS) {
        ocl->handle_kernel_arg_error(ret);
        cout << "Error: Failed to set kernel argument: run data!" << endl;
        exit(1);
    }

    //Final returned value
    ret = clSetKernelArg(ocl->kernel, numArg++, sizeof(cl_mem), &final_graph);
    if (ret != CL_SUCCESS) {
        ocl->handle_kernel_arg_error(ret);
        cout << "Error: Failed to set kernel argument: returned values!" << endl;
        exit(1);
    }

    //Sanity check
    ret = clSetKernelArg(ocl->kernel, numArg++, sizeof(cl_mem), &sanity_check_graph);
    if (ret != CL_SUCCESS) {
        ocl->handle_kernel_arg_error(ret);
        cout << "Error: Failed to set kernel argument: sanity check!" << endl;
        exit(1);
    }
}

void Fuzzer::handle_returned_data()
{
    cout
    << "Going through " << (local_item_size * 32)
    << " elements to test..." << endl;

    bool allOK = true;
    Emulator emulator;
    for(unsigned i = 0; i < 32 * local_item_size; i++) {

        //Calculate ourselves
        Output calculated_data;
        emulator.emulate(
            runData.start + i
            , calculated_data
        );

        //Get data on other end
        Output device_data;
        backroll_bits(device_data, final_main + (i/32)*NUM_OUTPUTS, i%32);

        //Check that they are the same
        bool thisOK = true;
        for(unsigned i2 = 0; i2 < NUM_OUTPUTS_BYTES; i2++) {
            if (device_data[i2] != calculated_data[i2]) {
                {
                    allOK = false;
                    thisOK = false;
                }
            }
        }

        if (!thisOK) {
            if (!thisOK) {
                cout << "Following data is WRONG!!!" << endl;
            }

            cout << "Keystart for this: "
            << (runData.start + i)
            << endl;

            cout
            << "Data here : "
            << print_data(calculated_data, NUM_OUTPUTS_BYTES) << endl
            << "Data there: " << print_data(device_data, NUM_OUTPUTS_BYTES)
            << endl;
        }
    }
    if (allOK) {
        cout << "All computed data is OK" << endl;
    } else {
        cout << "Some computed data is NOT OK!!" << endl;
    }
}

void Fuzzer::calc_enque_update()
{
    runData.start = 0;
    cout << "start value: " << runData.start << endl;
    runData.num_times = 1;

    cl_int ret;

    //Set runData
    ret = clEnqueueWriteBuffer(
        ocl->command_queue //The command queue (and its associate device) to enqueue at
        , runData_graph //Copy to this place
        , CL_TRUE //Blocking?
        , 0 //he offset in bytes in the buffer object to read from or write to
        , sizeof(RunData) //size of data to copy
        , &runData //pointer to data to copy
        , 0 //event wait list size
        , NULL //event wait list
        , NULL
    );

    if (ret != CL_SUCCESS) {
        ocl->decode_enqueue_readwrite_error(ret);
        cout << "Cannot enqueue write buffer" << endl;
        exit(-1);
    }

    cout << "Enqueuing kernel ..." << std::flush;
    size_t local_item_size = LOCAL_SIZE;
    ret = clEnqueueNDRangeKernel(
        ocl->command_queue //The command queue (and its associate device) to enqueue at
        , ocl->kernel //The kenel to enqueue
        , 1 //Dimension
        , NULL //Work offset
        , &local_item_size //Global work size
        , &local_item_size //NULL = automatic
        , 0
        , NULL
        , NULL
    );
    if (ret != CL_SUCCESS) {
        ocl->print_enqueue_problem(ret);
        exit(-1);
    }

    //Issue the command
    ret = clFlush(ocl->command_queue);
    if (ret != CL_SUCCESS) {
        cout << "Couldn't flush command queue" << endl;
        exit(-1);
    }
    cout << "Done. " << endl;
}

void Fuzzer::init_opencl()
{
    vector<string> filenames;
    filenames.push_back("commondata.h");
    filenames.push_back("calc.h");
    filenames.push_back("calc.c");
    filenames.push_back("kernel.c");

    string kernelname = "fuzz";

    ocl = new OpenCLHelper(
        1
        , 0 //the device number to use
        , optimize_compile //disable optimization in the compiler
    );
    ocl->setup_background(
        filenames
        , kernelname
    );
}

void Fuzzer::fuzz()
{
    assert(sharedData != NULL);
    init_opencl();

    final_main = new cl_uint[final_main_size];
    setup_graph_memory();

    set_kernel_args();
    calc_enque_update();
    sanityCheck.num_times_other_end = ~0;
    read_result();
    check_sanity_func();
    handle_returned_data();

    free_datastructs();
}

void Fuzzer::free_datastructs()
{
    ocl->release_all();
    free_graph_memory();
    free_main_memory();

    delete ocl;
}

void Fuzzer::check_sanity_func() const
{
    cout << "Num times on this end : " << (runData.num_times) << endl;
    cout << "Num times on other end: " << (sanityCheck.num_times_other_end) << endl;
    assert(sanityCheck.num_times_other_end == runData.num_times);
}

void Fuzzer::read_result()
{
    cl_int ret;
    memset(final_main, 0xff, final_main_size * sizeof(cl_uint));
    ret = clEnqueueReadBuffer(
        ocl->command_queue
        , final_graph
        , CL_TRUE
        , 0
        , final_main_size * sizeof(cl_uint)
        , final_main
        , 0
        , NULL
        , NULL
    );
    if (ret != CL_SUCCESS) {
        ocl->decode_enqueue_readwrite_error(ret);
        cout << "Cannot enqueue read buffer -- reading results" << endl;
        exit(-1);
    }

    ret = clEnqueueReadBuffer(
        ocl->command_queue //The command queue
        , sanity_check_graph //read from here
        , CL_TRUE //blocking read
        , 0 //offset of read
        , sizeof(SanityCheck) //amount of bytes to read
        , &sanityCheck //where to put it
        , 0
        , NULL
        , NULL
    );
    if (ret != CL_SUCCESS) {
        ocl->decode_enqueue_readwrite_error(ret);
        cout << "Cannot enqueue read buffer -- reading results" << endl;
        exit(-1);
    }

    cout
    << "Read back "
    << (local_item_size * sizeof(cl_uint) * 8)
    << " chains"
    << endl;
}

void Fuzzer::free_graph_memory()
{
    cl_int ret;

    ret = clReleaseMemObject(runData_graph);
    if (ret != CL_SUCCESS) {
        cout << "Couldn't free graph mem" << endl;
        exit(-1);
    }

    ret = clReleaseMemObject(final_graph);
    if (ret != CL_SUCCESS) {
        cout << "Couldn't free graph mem" << endl;
        exit(-1);
    }

    ret = clReleaseMemObject(sanity_check_graph);
    if (ret != CL_SUCCESS) {
        cout << "Couldn't free graph mem" << endl;
        exit(-1);
    }
}

void Fuzzer::free_main_memory()
{
    delete[] final_main;
}

void Fuzzer::set_shared_data(SharedData* _sharedData)
{
    sharedData = _sharedData;
}
