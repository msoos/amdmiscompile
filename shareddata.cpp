#include "shareddata.h"
#include "commondata.h"

#include <assert.h>
#include <iomanip>
#include <string>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

cl_platform_id SharedData::platform_id;
cl_device_id SharedData::device_id[10];
cl_uint SharedData::num_devices;

void SharedData::get_platform_and_devices(bool use_cpu)
{
    cl_int ret;

    //Get platform and device information --- only ONE platform can be selected
    platform_id = NULL;
    cl_uint ret_num_platforms;
    ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
    if (ret != CL_SUCCESS) {
        cout << "Could not get platform IDs" << endl;
        exit(-1);
    }
    cout << "Num platforms: " << ret_num_platforms << endl;

    char charbuffer[256];
    ret = clGetPlatformInfo( platform_id, CL_PLATFORM_NAME, sizeof(charbuffer), charbuffer, NULL);
    if (ret != CL_SUCCESS) {
        cerr << "Failed to get platform name" << endl;
        exit(-1);
    }
    cout << " Platform name: " << charbuffer << endl;
    string myplatformname = charbuffer;
    if (myplatformname.find("AMD") == string::npos
        && myplatformname.find("NVIDIA") == string::npos
    ) {
        cout
        << "ERROR: Your platform doesn't have AMD or NVIDIA in its name. Aborting."
        << endl;

        exit(-1);
    }

    ret = clGetPlatformInfo( platform_id, CL_PLATFORM_VERSION, sizeof(charbuffer), charbuffer, NULL);
    if (ret != CL_SUCCESS) {
        cerr << "Failed to get platform version" << endl;
        exit(-1);
    }
    cout << " Platform version: " << charbuffer << endl;

    //Select GPU device. Only ONE at the moment
    ret = clGetDeviceIDs(platform_id
        , (use_cpu ? CL_DEVICE_TYPE_CPU : CL_DEVICE_TYPE_GPU)
        , 10
        , device_id
        , &num_devices
    );

    if (ret != CL_SUCCESS) {
        cout << "Could not get device IDs: ";
        switch(ret) {
            case CL_INVALID_PLATFORM:
                cout << "Not a valid platform!" << endl;
                break;
            case CL_INVALID_DEVICE_TYPE:
                cout << "Not a valid device type!" << endl;
                break;
            case CL_INVALID_VALUE:
                cout << "Wrong method call -- invalid device value" << endl;
                break;
            case CL_DEVICE_NOT_FOUND:
                cout << "No GPU device found" << endl;
                break;
            default:
                cout << "General (unknown) error!" << endl;
                break;
        }
        exit(-1);
    }
    if (num_devices < 1) {
        cout << "No GPU device installed, exiting. " << endl;
        exit(-1);
    }
    cout << "Num GPU device(s) recognized: " << num_devices << endl;
}
