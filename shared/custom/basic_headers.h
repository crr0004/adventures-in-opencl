#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include "common.h"
 

// Silence
#define CL_TARGET_OPENCL_VERSION 120

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include "clext.h"