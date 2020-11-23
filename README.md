# adventures-in-opencl
A accompanying repo for my writing about OpenCL - https://dev.to/crr0004/series/1481

# Building
You will need a driver that implements OpenCL 2.0 and cmake.

This was compiled using clang. It should automatically pick up your primary
OpenCL vendor and use that.

## Headers

The OpenCL headers are included in this repo under shared, if CMake doesn't find them, you can override the directory with "-DOPENCL_INCLUDE_DIRS=../../shared/" when building from one of the build directories. Change the path if it is different for you.

When running it will print out what device is being used but if your
vendor implementation is broken it can crash your driver.

# Running
Run each sample from the source directory as it needs to pick up the kernel
files

# Used libraries
- Catch2 https://github.com/catchorg/Catch2
- Google Benchmark https://github.com/google/benchmark/

See LICENSE file for addition of respective licenses
