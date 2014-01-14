fuzzer: calc.c calc.h commondata.h convert_and_print.h emulator.cpp emulator.h fuzzer.cpp fuzzer.h fuzzer_main.cpp kernel.c openclhelper.cpp openclhelper.h shareddata.cpp shareddata.h
	g++ -Wall fuzzer.cpp fuzzer_main.cpp emulator.cpp shareddata.cpp openclhelper.cpp -lrt -lOpenCL -o fuzzer
