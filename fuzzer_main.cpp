#include "fuzzer.h"
#include "shareddata.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;

const char* hasPrefix(const char* str, const char* prefix)
{
    int len = strlen(prefix);
    if (strncmp(str, prefix, len) == 0)
        return str + len;
    else
        return NULL;
}

void print_usage()
{
    cout
    << "AMD bug demonstrator. Supported options: " << endl
    << "--cpu        : use the CPU for computation" << endl
    << "--unoptimize : do not optimize the compilation" << endl
    << endl;
}

int main(int argc, char *argv[])
{
    bool use_cpu = 0;
    int optimize_compile = 1;

    const char* value;
    int j = 0;

    for (int i = 0; i < argc; i++) {
        if ((value = hasPrefix(argv[i], "--unoptimize"))) {
            optimize_compile = 0;
        } else if ((value = hasPrefix(argv[i], "--cpu"))) {
            use_cpu = true;
        } else if (
            (value = hasPrefix(argv[i], "--help")) || (value = hasPrefix(argv[i], "-h"))
        ) {
            print_usage();
            exit(0);
        } else if (strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0) {
            cout << "Error, unknown flag: " << argv[i] << endl;
            print_usage();
            exit(-1);
        } else {
            argv[j++] = argv[i];
        }
    }
    argc = j;

    cout << "Options you gave: " << endl;
    if (use_cpu) {
        cout << "- Using CPU instead of GPU" << endl;
    } else {
        cout << "- Using GPU for computation" << endl;
    }

    if (optimize_compile) {
        cout << "- Optimizing compilation" << endl;
    } else {
        cout << "- Not optimizing compilation" << endl;
    }

    SharedData::get_platform_and_devices(use_cpu);
    SharedData sharedData;
    Fuzzer* fuzzer = new Fuzzer(optimize_compile);
    fuzzer->set_shared_data(&sharedData);
    int ret = fuzzer->fuzz();
    delete fuzzer;
    cout << "Finished." << endl;

    return ret;
}
