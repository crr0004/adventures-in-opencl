#include <stdio.h>
#include <stdlib.h>
#include <cstring>
 

// Silence
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include "clext.h"
 
#define MAX_SOURCE_SIZE (0x100000)

void printPlatformName(cl_platform_id id){
	char outBuf[256];
	size_t outBufWritten = 0;
	clGetPlatformInfo(id, CL_PLATFORM_NAME, 256, outBuf, &outBufWritten);
	fprintf(stdout, "Platform name %s.\n", outBuf);
}
void printDeviceInfo(cl_device_id id){
    char buf[128];
        clGetDeviceInfo(id, CL_DEVICE_NAME, 128, buf, NULL);
        fprintf(stdout, "Device %s supports ", buf);

        clGetDeviceInfo(id, CL_DEVICE_VERSION, 128, buf, NULL);
        fprintf(stdout, "%s\n", buf);

}

void printDevicesOnPlatform(cl_platform_id platform){

    cl_uint num_devices, i;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
	fprintf(stdout, "Found %d devices.\n", num_devices);

    cl_device_id devices[num_devices];
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

    for (i = 0; i < num_devices; i++) {
		printDeviceInfo(devices[i]);
    }

}
 
int main(void) {
    // Create the two input vectors
    int i;
    const int LIST_SIZE = 1024;
    int *A = (int*)malloc(sizeof(int)*LIST_SIZE);
    int *B = (int*)malloc(sizeof(int)*LIST_SIZE);
    for(i = 0; i < LIST_SIZE; i++) {
        A[i] = i;
        B[i] = LIST_SIZE - i;
    }
 
    // Load the kernel source code into the array source_str
    FILE *fp;
    char *source_str;
    size_t source_size;
 
    fp = fopen("kernel.cl", "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose( fp );
 
    // Get platform and device information
    cl_platform_id platform_id[2];
    cl_device_id device_id = NULL;   
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret = clGetPlatformIDs(2, platform_id, &ret_num_platforms);
    ret = clGetDeviceIDs( platform_id[0], CL_DEVICE_TYPE_DEFAULT, 1, 
            &device_id, &ret_num_devices);

    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}

	char buf[128];
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 128, buf, NULL);
	fprintf(stdout, "Device %s supports.\n", buf);

	printPlatformName(platform_id[0]);
 
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}

    ret = clGetDeviceIDs( platform_id[0], CL_DEVICE_TYPE_DEFAULT, 1, 
            &device_id, &ret_num_devices);

	char outBuf[256];
	size_t outBufWritten = 0;
	clGetPlatformInfo(platform_id[0], CL_PLATFORM_NAME, 256, outBuf, &outBufWritten);
	fprintf(stdout, "Using %d Platform name %s.\n", i, outBuf);
	printDeviceInfo(device_id);
 
	clCheckError(ret);



    // Create an OpenCL context
    cl_context context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &ret);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}

	cl_device_id used_device_id;
	ret = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof used_device_id, &used_device_id, NULL);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
	std::memset(buf, 0, sizeof buf);
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 128, buf, NULL);
	fprintf(stdout, "Using %s for context device.\n", buf);
	
 
    // Create a command queue
    cl_command_queue command_queue = clCreateCommandQueue(context, device_id, 0, &ret);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
 
    // Create memory buffers on the device for each vector 
    cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, 
            LIST_SIZE * sizeof(int), NULL, &ret);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
    cl_mem b_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY,
            LIST_SIZE * sizeof(int), NULL, &ret);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
    cl_mem c_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 
            LIST_SIZE * sizeof(int), NULL, &ret);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
 
    // Copy the lists A and B to their respective memory buffers
    ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0,
            LIST_SIZE * sizeof(int), A, 0, NULL, NULL);
	clCheckError(ret);

    ret = clEnqueueWriteBuffer(command_queue, b_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(int), B, 0, NULL, NULL);
	clCheckError(ret);
 
    // Create a program from the kernel source
    cl_program program = clCreateProgramWithSource(context, 1, 
            (const char **)&source_str, (const size_t *)&source_size, &ret);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
 
    // Build the program
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	clCheckError(ret);
 
    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "vector_add", &ret);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
 
    // Set the arguments of the kernel
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
    if(ret != CL_SUCCESS){
		clCheckError(ret);
		exit(1);
	}
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&c_mem_obj);
	clCheckError(ret);
 
    // Execute the OpenCL kernel on the list
    size_t global_item_size = LIST_SIZE; // Process the entire lists
    size_t local_item_size = 32; // Divide work items into groups of 64
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 
            &global_item_size, &local_item_size, 0, NULL, NULL);
	clCheckError(ret);
 
    // Read the memory buffer C on the device to the local variable C
    int *C = (int*)malloc(sizeof(int)*LIST_SIZE);
    ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, 
            LIST_SIZE * sizeof(int), C, 0, NULL, NULL);
	clCheckError(ret);
 
    // Display the result to the screen
    //for(i = 0; i < LIST_SIZE; i++)
     //   printf("%d + %d = %d\n", A[i], B[i], C[i]);
 
    // Clean up
    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(a_mem_obj);
    ret = clReleaseMemObject(b_mem_obj);
    ret = clReleaseMemObject(c_mem_obj);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    free(A);
    free(B);
    free(C);
    return 0;
}
