AMD OpenCL miscompile demo
=============

This system demonstrates that the AMD OpenCL compiler miscompiles kernel code. To execute:

```
$ make
$ ./fuzzer --cpu
$ ./fuzzer --unoptimize
$ ./fuzzer
```

The first two should indicate that everything went fine, the OpenCL system computed the values correctly if the code is compiled to run on the CPU or it's compiled in an unoptimized way. The last one, using OpenCL code compiled in optimized mode running on the GPU, shows that the compiler miscompiled the kernel and the data calculated is thus wrong.

In other words, the OpenCL compiler for AMD cannot correctly compile the small kernel code in `calc.c`. If you remove the variable `B2` it will work, however. The code is really small and quite easy to check that it is correct. 

How to 'fix' the miscompile
-------------
Simply remove the bitwise inversion from 'c':

```
void clk_sr_pre(struct Calc* calc)
{
    unsigned a, b, c;
    a = calc->dat[1];
    b = calc->dat[26];
    c = calc->dat[23];
    unsigned B2 = ( a & (~b) );
    //unsigned A = (  b   & (~c) ); //original
    unsigned A = (  b   & (c) ); //'fixed'
    A |= B2;

    calc->sig1 = A;
}
```

then:

```
$ make
$ ./fuzzer
```

It should now indicate that everything is fine. This indicates that it is indeed the OpenCL compiler that is responsible for the bug.


Future work
-------------
The AMD OpenCL compiler has many other bugs too, but this only dmonstrates one miscompilation bug. I intend to make a fuzzer out of it. It shouldn't be very hard and it will probably find many bugs. Given that WebCL may become mainstream, this could become a major headache.
