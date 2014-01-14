#include <fstream>
#include <sstream>

#include "openclhelper.h"
#include "shareddata.h"

void mynotify(const char* errinfo, const void *private_info, size_t cb, void* user_data)
{
    cout << "ERROR IN CONTEXT: " << errinfo << endl;
    exit(-1);
}

void OpenCLHelper::get_device_and_kernel_data(
  size_t workgroup_multiplier
) {
    cl_int ret;

    // Determine sizes
    size_t max_workgroup_size;
    ret = clGetDeviceInfo(
        SharedData::device_id[device_num]
        , CL_DEVICE_MAX_WORK_GROUP_SIZE
        , sizeof(max_workgroup_size)
        , &max_workgroup_size
        ,  NULL
    );
    if (ret != CL_SUCCESS) {
        cout << "Couldn't get device-specific maximum workgroup size" << endl;
        exit(-1);
    }
    if (verbosity > 1)
        cout << "Device-specific max workgroup size: " << max_workgroup_size << endl;

    size_t max_kernel_workgroup_size;
    ret = clGetKernelWorkGroupInfo(
        kernel
        , SharedData::device_id[device_num]
        , CL_KERNEL_WORK_GROUP_SIZE
        , sizeof(max_kernel_workgroup_size)
        , &max_kernel_workgroup_size
        , NULL
    );
    if (ret != CL_SUCCESS) {
        cout << "Couldn't query kernel-specific maximum workgroup size" << endl;
        exit(-1);
    }

    if (verbosity > 0)
        cout << "Kernel-specific max workgroup size: " << max_kernel_workgroup_size << endl;

    cl_ulong max_local_mem_size;
    ret = clGetKernelWorkGroupInfo(
        kernel
        , SharedData::device_id[device_num]
        , CL_KERNEL_LOCAL_MEM_SIZE
        , sizeof(max_local_mem_size)
        , &max_local_mem_size
        , NULL
    );
    if (ret != CL_SUCCESS) {
        cout << "Couldn't query local memory used by kernel" << endl;
        exit(-1);
    }
    if (verbosity > 0)
        cout << "Local memory used by kernel: " << max_local_mem_size << endl;

    cl_ulong local_mem_size;
    ret = clGetDeviceInfo(
        SharedData::device_id[device_num]
        , CL_DEVICE_LOCAL_MEM_SIZE
        , sizeof(local_mem_size)
        , &local_mem_size
        , NULL
    );
    if (ret != CL_SUCCESS) {
        cout << "Couldn't query local memory of device" << endl;
        exit(-1);
    }
    if (verbosity > 1)
        cout << "Local memory on device (in bytes): " << local_mem_size << endl;

    unsigned max_compute_units;
    ret = clGetDeviceInfo(
        SharedData::device_id[device_num]
        , CL_DEVICE_MAX_COMPUTE_UNITS
        , sizeof(max_compute_units)
        , &max_compute_units
        , NULL
    );
    if (ret != CL_SUCCESS) {
        cout << "Couldn't query max compute units of device" << endl;
        exit(-1);
    }
    //if (verbosity > 1)
        cout << "[opencl] Max compute units on device: " << max_compute_units << endl;

    if (workgroup_multiplier != std::numeric_limits<size_t>::max()
        && workgroup_multiplier % max_compute_units != 0
    ) {
        cout
        << "ERROR: Workgroup multipler (-m or --workmult) is not a multiple of"
        << " the number of compute units on the device:" << endl
        << "--> Compute Units: " << max_compute_units << endl
        << "--> Multipler    : " << workgroup_multiplier << endl
        << " We sugest a 10-20x multiplier over compute units for optimal performance" << endl
        << " Note that if multiplier is not divisible by the number of compute units you *always* get suboptimal performance"
        << endl;
        exit(-1);
    }

    unsigned long long max_constant_buffer_size;
    ret = clGetDeviceInfo(
        SharedData::device_id[device_num]
        , CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE
        , sizeof(max_constant_buffer_size)
        , &max_constant_buffer_size
        , NULL
    );
    if (ret != CL_SUCCESS) {
        cout << "Couldn't query max constant memory size of device" << endl;
        exit(-1);
    }
    if (verbosity > 1)
        cout << "Max constant memory on device (in bytes): " << max_constant_buffer_size << endl;
}

string OpenCLHelper::read_source(const string filename)
{
    std::ifstream ifile;
    ifile.open(filename.c_str());
    if (!ifile) {
        cout << "Failed to load kernel file '" << filename << "'" << endl;
        exit(1);
    }
    string source_str((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
    ifile.close();

    return source_str;
}

void OpenCLHelper::read_sources(const vector<string>& filenames)
{
    //sources.push_back(read_source("example.cl"));

    for(vector<string>::const_iterator
        it = filenames.begin(), end = filenames.end()
        ; it != end
        ; it++
    ) {
        sources.push_back(read_source(*it));
    }
}

void OpenCLHelper::get_source_sizes()
{

    for(vector<string>::const_iterator
        it = sources.begin(), end = sources.end()
        ; it != end
        ; it++
    ) {
        source_sizes.push_back(it->size());
    }
}

void OpenCLHelper::build_program()
{
    cl_int ret;

    const char** tmp;
    tmp = (const char**)malloc(sources.size()*sizeof(const char*));
    for (unsigned int i = 0; i < sources.size(); i++) {
        tmp[i] = sources[i].c_str();
    }
    program = clCreateProgramWithSource(context, (cl_uint)sources.size(), tmp, &source_sizes[0], &ret);

    if (ret != CL_SUCCESS) {
        cout << "Error loading kernel from file: " << endl;
        switch(ret) {
            case CL_INVALID_CONTEXT:
                cout
                << "context is not a valid context"
                << endl;
                break;
            case CL_INVALID_VALUE:
                cout
                << "count is zero or if strings or any entry in strings is NULL"
                << endl;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                cout << "there is a failure to allocate resources required by the OpenCL implementation on the host."
                << endl;
                break;
            default:
                cout << "Some undecodable error"
                << endl;
                break;

        }
        exit(-1);
    }

    // Build the program
    std::stringstream options;
    //options
    //<< "-cl-strict-aliasing"
    //<< " -save-temps"
    //;

    if (!enable_opt) {
        options << " -cl-opt-disable";
    }

    //const char* options = "-cl-opt-disable";
    cout << "[opencl] device number: " << device_num << endl;
    ret = clBuildProgram(program, 1, &(SharedData::device_id[device_num]), options.str().c_str(), NULL, NULL);
    if (ret != CL_SUCCESS) {
        cout << "Error while building kernel: ";
        switch (ret) {
            case CL_INVALID_PROGRAM:
                cout << "Incorrect program" << endl;
                break;
            case CL_INVALID_VALUE:
                cout << "Incorrect parameter to clBuildProgram" << endl;
                break;
            case CL_INVALID_DEVICE:
                cout << "Incorrect device for program" << endl;
                break;
            case CL_INVALID_BUILD_OPTIONS:
                cout << "Incorrect build options for program" << endl;
                break;
            case CL_COMPILER_NOT_AVAILABLE:
                cout << "OpenCL compiler not available" << endl;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                cout << "Host out of memory while building" << endl;
                break;
            case CL_BUILD_PROGRAM_FAILURE:
                cout << "Kernel program failed to build" << endl;
                break;
            default:
                cout << "General error" << endl;
                break;
        }
        print_build_log();

        exit(-1);
    }

    cl_int status;
    ret = clGetProgramBuildInfo(
        program, SharedData::device_id[device_num]
        , CL_PROGRAM_BUILD_STATUS
        , sizeof(status)
        , &status
        , NULL
    );
    if (ret != CL_SUCCESS) {
        cout << "Cannot query build status" << endl;
        exit(-1);
    }

    if (verbosity > 0)
        cout << "build status: ";

    switch(status) {
        case CL_BUILD_NONE:
            cout << "no build has been performed on the specified program object for device" << endl;
            break;
        case CL_BUILD_ERROR:
            cout << "specified program object for device generated an error." << endl;
            break;
        case CL_BUILD_SUCCESS:
            if (verbosity > 0)
                cout << "specified program object for device was successful." << endl;
            break;
        case CL_BUILD_IN_PROGRESS:
            cout << "specified program object for device has not finished." << endl;
            break;
    }
    print_build_log();

    //Get bulit program size
    size_t program_size;
    ret = clGetProgramInfo(
        program
        , CL_PROGRAM_BINARY_SIZES
        , sizeof(program_size)
        , &program_size
        , NULL
    );
    if (verbosity > 1)
        cout << "Built program binary size: " << ((double)program_size/1024) << " KB" << endl;
}

void OpenCLHelper::handle_kernel_arg_error(cl_int ret)
{
    switch(ret) {
        case CL_INVALID_KERNEL:
            cout << "kernel is not a valid kernel object." << endl;
            break;
        case CL_INVALID_ARG_INDEX:
            cout << "arg_index is not a valid argument index." << endl;
            break;
        case CL_INVALID_ARG_VALUE:
            cout << "arg_value specified is NULL for an argument that is not declared with the __local qualifier or vice-versa." << endl;
            break;
        case CL_INVALID_MEM_OBJECT:
            cout << "argument declared to be a memory object when the specified arg_value is not a valid memory object." << endl;
            break;
        case CL_INVALID_SAMPLER:
            cout << "argument declared to be of type sampler_t when the specified arg_value is not a valid sampler object." << endl;
            break;
        case CL_INVALID_ARG_SIZE:
            cout << "arg_size does not match the size of the data type for an argument that is not a memory object or if the argument is a memory object and arg_size != sizeof(cl_mem) or if arg_size is zero and the argument is declared with the __local qualifier or if the argument is a sampler and arg_size !=sizeof(cl_sampler)." << endl;
            break;
    }
}

void OpenCLHelper::print_enqueue_problem(const cl_int ret)
{
    switch(ret) {
        case CL_INVALID_PROGRAM_EXECUTABLE :
            cout << "no successfully built program executable available for device associated with command_queue" << endl;
            break;
        case CL_INVALID_COMMAND_QUEUE :
             cout << "command_queue is not a valid command-queue" << endl;
             break;
        case CL_INVALID_KERNEL :
            cout << "if kernel is not a valid kernel object" << endl;
            break;
        case CL_INVALID_CONTEXT:
            cout << "context associated with command_queue and kernel is not the same or if the context associated with command_queue and events in event_wait_list are not the same" << endl;
            break;
        case CL_INVALID_KERNEL_ARGS:
            cout << "the kernel argument values have not been specified" << endl;
            break;
        case CL_INVALID_WORK_DIMENSION:
            cout << "if work_dim is not a valid value (i.e. a value between 1 and 3)" << endl;
            break;
        case CL_INVALID_GLOBAL_WORK_SIZE:
            cout << "global_work_size is NULL, or if any of the values specified in global_work_size[0], ... global_work_size[work_dim – 1] are 0 or exceed the range given by the sizeof(size_t) for the device on which the kernel execution will be enqueued" << endl;
            break;
        case CL_INVALID_WORK_GROUP_SIZE:
            cout << "local_work_size is specified and number of work-items specified by global_work_size is not evenly divisible by size of work-group given by local_work_size or does not match the work-group size specified for kernel using the __attribute__((reqd_work_group_size(X, Y, Z))) qualifier in program source." << endl;

            cout << endl << "OR local_work_size is specified and the total number of work-items in the work-group computed as local_work_size[0] * ... local_work_size[work_dim – 1] is greater than the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in table 4.3." << endl;

            cout << endl << "OR local_work_size is NULL and the __attribute__((reqd_work_group_size(X, Y, Z))) qualifier is used to declare the work-group size for kernel in the program source" << endl;
            break;
        case CL_INVALID_WORK_ITEM_SIZE:
            cout << "the number of work-items specified in any of local_work_size[0], ... local_work_size[work_dim – 1] is greater than the corresponding values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], .... CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim – 1]. CL_INVALID_GLOBAL_OFFSET if global_work_offset is not NULL." << endl;
            break;
        case CL_OUT_OF_RESOURCES:
            cout << "there is a failure to queue the execution instance of kernel on the command-queue because of insufficient resources needed to execute the kernel. For example, the explicitly specified local_work_size causes a failure to execute the kernel because of insufficient resources such as registers or local memory. Another example would be the number of read-only image args used in kernel exceed the CL_DEVICE_MAX_READ_IMAGE_ARGS value for device or the number of write-only image args used in kernel exceed the CL_DEVICE_MAX_WRITE_IMAGE_ARGS value for device or the number of samplers used in kernel exceed CL_DEVICE_MAX_SAMPLERS for device." << endl;
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            cout << "there is a failure to allocate memory for data store associated with image or buffer objects specified as arguments to kernel." << endl;
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            cout << "event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events." << endl;
            break;
        case  CL_OUT_OF_HOST_MEMORY:
            cout << "there is a failure to allocate resources required by the OpenCL implementation on the host." << endl;
            break;
    }
}

void OpenCLHelper::setup_device_context()
{
    cl_int ret;

    // Create an OpenCL context.
    //For managing memory, command queues, program&kernel objects
    context = clCreateContext( NULL, 1, &(SharedData::device_id[device_num]), mynotify, NULL, &ret);
    if (ret != CL_SUCCESS) {
        cout << "Error creating context" << endl;
        switch(ret) {
            case  CL_INVALID_PLATFORM:
                cout
                << "properties is NULL and no platform could be selected"
                << " or if platform value specified in properties is not a valid platform"
                << endl;
                break;
            case CL_INVALID_VALUE:
                cout
                << "context property name in properties is not a supported property name"
                << " or if devices is NULL; if num_devices is equal to zero;"
                << " or if pfn_notify is NULL but user_data is not NULL."
                << endl;
                break;
            case CL_INVALID_DEVICE:
                cout
                << "devices contains an invalid device or are not associated with the specified platform"
                << endl;
                break;
            case CL_DEVICE_NOT_AVAILABLE:
                cout
                << "a device in devices is currently not available even though"
                << " the device was returned by clGetDeviceIDs."
                << endl;
                break;
            case CL_OUT_OF_HOST_MEMORY:
                cout <<
                "there is a failure to allocate resources required by the OpenCL implementation on the host."
                << endl;
                break;
        }
        exit(-1);
    }
}

void OpenCLHelper::decode_enqueue_readwrite_error(const cl_int err)
{
    cout << "ERROR: ";
    switch(err) {
        case CL_INVALID_COMMAND_QUEUE:
            cout << "command_queue is not a valid command-queue." << endl;
            break;
        case CL_INVALID_CONTEXT:
            cout << "the context associated with command_queue and buffer arenot the same or if the context associated with command_queue and events in event_wait_list are not the same." << endl;
            break;
        case CL_INVALID_MEM_OBJECT:
            cout << "buffer is not a valid buffer object." << endl;
            break;
        case CL_INVALID_VALUE:
            cout << "the region being read or written specified by (offset, cb) is out of bounds or if ptr is a NULL value." << endl;
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            cout << "event_wait_list is NULL and num_events_in_wait_list > 0, or event_wait_list is not NULL and num_events_in_wait_list is 0, or if event objects in event_wait_list are not valid events." << endl;
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            cout << "there is a failure to allocate memory for data store associated with buffer." << endl;
            break;
        case CL_OUT_OF_HOST_MEMORY:
            cout << "if there is a failure to allocate resource" << endl;
            break;
        default:
            cout << "cannot decode reason for error" << endl;
    }
}

void OpenCLHelper::print_build_log()
{
    cl_int ret;
    char log[2048];

    size_t retsize;
    ret = clGetProgramBuildInfo(
        program
        , SharedData::device_id[device_num]
        , CL_PROGRAM_BUILD_LOG
        , sizeof(log)
        , log
        , &retsize
    );
    if (ret != CL_SUCCESS) {
        cout << "Cannot query build log" << endl;
        exit(-1);
    }

    cout << "---- Build log ------ " << endl
    << log << endl
    << "------ Build log end ---------" << endl;
}

void OpenCLHelper::setup_background(
    const vector<string>& filenames
    , string kernelname
) {
    cl_int ret;

    //Setup device
    setup_device_context();

    // Create a command queue
    command_queue = clCreateCommandQueue(
        context
        , SharedData::device_id[device_num]
        , 0
        , &ret
    );
    if (ret != CL_SUCCESS) {
        cout << "Error creating command queue" << endl;
        exit(-1);
    }
    if (verbosity > 0)
        cout << "Created command queue" << endl;

    //Build program
    if (verbosity > 0)
        cout << "Building program.." << endl;

    // Load the kernel source code into string source_str
    read_sources(filenames);
    get_source_sizes();
    build_program();

    //Create the OpenCL kernel from program
    kernel = clCreateKernel(program, kernelname.c_str(), &ret);
    if (ret != CL_SUCCESS)  {
        cout << "Cannot create kernel object! Maybe kernel name was wrongly given?" << endl;
        exit(-1);
    }
    get_device_and_kernel_data();

    /////////////////////////////
    //Setup parameters
    if (verbosity > 0)
        cout << "Workgroup size: " << LOCAL_SIZE << endl;
}

void OpenCLHelper::release_all()
{
    cl_int ret;

    ret = clReleaseKernel(kernel);
    if (ret != CL_SUCCESS) {
        cout << "Couldn't release kernel" << endl;
        exit(-1);
    }
    ret = clReleaseProgram(program);
    if (ret != CL_SUCCESS) {
        cout << "Couldn't release program" << endl;
        exit(-1);
    }

    ret = clReleaseCommandQueue(command_queue);
    if (ret != CL_SUCCESS) {
        cout << "Couldn't release command queue" << endl;
        exit(-1);
    }
    ret = clReleaseContext(context);
    if (ret != CL_SUCCESS) {
        cout << "Couldn't release context" << endl;
        exit(-1);
    }
}

