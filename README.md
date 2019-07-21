# adventures-in-opencl
A accompanying repo for my writing about OpenCL

# Building
You will need a driver that implements OpenCL 2.0 and cmake.

This was compiled using clang. It should automatically pick up your primary
OpenCL vendor and use that.

When running it will print out what device is being used but if your
vendor implementation is broken it can crash your driver.

# Running
Run each sample from the source directory as it needs to pick up the kernel
files
