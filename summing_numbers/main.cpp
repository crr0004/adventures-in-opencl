#include "basic_headers.h"
// #define CATCH_CONFIG_MAIN
// #define CATCH_CONFIG_COLOUR_NONE

#include <fstream>
#include <string>
#include <iostream>
#include <cmath>
#include <benchmark/benchmark.h>
// #include "catch.hpp"

void printPlatformName(cl_platform_id id) {
	char outBuf[256];
	size_t outBufWritten = 0;
	clGetPlatformInfo(id, CL_PLATFORM_NAME, 256, outBuf, &outBufWritten);
	fprintf(stdout, "Platform name %s.\n", outBuf);
}
void printDeviceInfo(cl_device_id id) {
	char buf[128];
	clGetDeviceInfo(id, CL_DEVICE_NAME, 128, buf, NULL);
	fprintf(stdout, "Device %s supports ", buf);

	clGetDeviceInfo(id, CL_DEVICE_VERSION, 128, buf, NULL);
	fprintf(stdout, "%s\n", buf);
}

void printDevicesOnPlatform(cl_platform_id platform) {
	cl_uint num_devices, i = -1;
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
	// fprintf(stdout, "Found %d devices.\n", num_devices);

	cl_device_id devices[num_devices];
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

	for(i = 0; i < num_devices; i++) {
		printDeviceInfo(devices[i]);
	}
}
cl_context setupContext(cl_device_id *usedDeviceId) {
	// Setup the variables for context setup
	size_t platformsToGet = 1;
	cl_platform_id platform_id[platformsToGet];
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_uint deviceSelection = 0;

	// Grab the platforms
	cl_int ret = clGetPlatformIDs(platformsToGet, platform_id, &ret_num_platforms);
	// fprintf(stdout, "%d number of platforms\n", ret_num_platforms);

	// Grab the second device
	ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	if(ret != CL_SUCCESS) {
		clCheckError(ret);
		exit(1);
	}

	// Dump query information from device
	char buf[128];
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 128, buf, NULL);
	// fprintf(stdout, "Device %s supports.\n", buf);

	for(int i = 0; i < ret_num_platforms; i++){
		// printPlatformName(platform_id[i]);
	}

	if(ret != CL_SUCCESS) {
		clCheckError(ret);
		exit(1);
	}

	// Create an OpenCL context
	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	if(ret != CL_SUCCESS) {
		clCheckError(ret);
		exit(1);
	}

	// Echo name of device in use for the context
	cl_device_id used_device_id;
	ret = clGetContextInfo(context, CL_CONTEXT_DEVICES, sizeof used_device_id, &used_device_id, NULL);
	if(ret != CL_SUCCESS) {
		clCheckError(ret);
		exit(1);
	}
	std::memset(buf, 0, sizeof buf);
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 128, buf, NULL);
	// fprintf(stdout, "Using %s for context device.\n", buf);

	if(usedDeviceId != nullptr){
		*usedDeviceId = used_device_id;
	}

	return context;
}



std::string readFile(const char* fileName, size_t *fileSize){
	std::ifstream a = std::ifstream(fileName, std::ios::in);

	// Jump to the end of the file so we can know how big it is
	a.seekg(0, std::ios::end);

	// See how many characters are in the file
	std::streampos size = a.tellg();
	// Create an empty string to read into
	std::string programSource = std::string(size, '\0');

	// Go back to the start of the file
	a.seekg(0, std::ios::beg);
	
	// Read in the file
	a.read(&programSource[0], size);
	// Close of the file handle
	a.close();

	*fileSize = size;
	return programSource;
}


cl_kernel createKernel(const char* fileName, const char* entryPoint, cl_context context, cl_device_id deviceId){

	size_t fileSize;
	std::string programSource = readFile(fileName, &fileSize);

	cl_int ret;
	const char* sourceStr = programSource.c_str();
	cl_program program =
	clCreateProgramWithSource(context, 1, (const char **)&sourceStr, (const size_t *)&fileSize, &ret);
	clCheckError(ret);

	// Build the program
    ret = clBuildProgram(program, 1, &deviceId, NULL, NULL, NULL);
	if(ret != CL_SUCCESS){
		size_t bufSize = 1024;
		char buf[bufSize];
		size_t returnedBuf = 0;

		clGetProgramBuildInfo(
			program,
			deviceId,
			CL_PROGRAM_BUILD_LOG,
			bufSize,
			&buf,
			&returnedBuf
		);
		if(returnedBuf == bufSize){
			std::cout << "More log entry is probably available for the program build" << std::endl;
		}
		std::cout << "Log entry for last program build: " << std::endl << buf << std::endl;
	}
	clCheckError(ret);
 
    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, entryPoint, &ret);
	clCheckError(ret);

	return kernel;
}

template<typename T> cl_mem setBufferIntoKernel(
	cl_context context, 
	T *numbers, 
	size_t size,
	cl_kernel kernel, 
	cl_command_queue commandQueue,
	cl_uint index
	){

    cl_int ret;
	cl_mem memObject = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(T), NULL, &ret);
	clCheckError(ret);

	ret = clEnqueueWriteBuffer(commandQueue, memObject, CL_TRUE, 0, size * sizeof(T), numbers, 0, NULL, NULL);
	clCheckError(ret);

	ret = clSetKernelArg(kernel, index, sizeof(cl_mem), (void *)&memObject);
	clCheckError(ret);

	return memObject;
}

template<typename T> cl_mem createBuffer(
	cl_context context, 
	size_t size,
	cl_kernel kernel, 
	cl_command_queue commandQueue,
	cl_uint index
){
	cl_int ret;
	cl_mem memObject =
	clCreateBuffer(context, CL_MEM_READ_WRITE, size * sizeof(T), NULL, &ret);
	clCheckError(ret);

	ret = clSetKernelArg(kernel, index, sizeof(cl_mem), (void *)&memObject);
	clCheckError(ret);

	return memObject;
}


// some openCL API objects
cl_context context;
cl_mem numbersMemObj; // buffer memory pointer
cl_kernel kernel; // kernel pointer
cl_device_id deviceId; // device we are going to use
cl_command_queue commandQueue;
cl_mem numberoutMemObj;
cl_int ret; // error number holder
cl_event kernelEnqueueToWaitFor; // a sync event to wait for
int *numbers;
int *numbersCopy;

void cleanContext(cl_context context){// Just grab the kernel program so we can release it
	cl_program program;
	clGetKernelInfo(kernel, CL_KERNEL_PROGRAM, sizeof(cl_program), &program, NULL);

	clFlush(commandQueue);
	clFinish(commandQueue);
	clReleaseEvent(kernelEnqueueToWaitFor);
	clReleaseProgram(program);
	clReleaseKernel(kernel);
	clReleaseMemObject(numberoutMemObj);
	clReleaseMemObject(numbersMemObj);
	clReleaseCommandQueue(commandQueue);
	clReleaseDevice(deviceId);
	clReleaseContext(context);
}

int setup(const size_t numberRange){

	// Create the buffer we are going to operate on
//	const size_t numberRange = 4*std::exp2(25);
	numbers = new int[numberRange];

	// create the buffer we are going to write the results into
	size_t bufSize = numberRange/4; // how many results we want
	numbersCopy = new int[bufSize];

	// fill in the numbers
	for(int i = 0; i < numberRange; i++) {
		numbers[i] = i;
	}

	// Create and setup the context to and assign the deviceId we will be using
	context = setupContext(&deviceId);

	// Create a command queue to push commands into
	commandQueue = clCreateCommandQueue(context, deviceId, 0, &ret);
	clCheckError(ret);

	// Create our compute kernel for the device, context and the entry point
	kernel = createKernel(
		"kernel.cl", // filepath to kernel source
		"add", // entry point name
		context, // opencl context
		deviceId // device to build against
		);

	// Write the buffer of numbers into the memory space the kernel will access
	return 0;
}
int runAdd(const size_t bufSize, size_t division) {
	// Tell the device that we want to run the kernel, and how it's compute space should be divided up
	const size_t localWorkgroupSize = 32; // how big each workgroup should be
	const size_t globalWorkSize = bufSize/division; // total index space for the kernel

	numbersMemObj = setBufferIntoKernel<int>(context, // context
	                                    numbers, // buffer
	                                    bufSize,
	                                    kernel, // kernel to set into
	                                    commandQueue,
	                                    0 // argument index in the kernel (left to right, zero indexed)
	);
	//Create a buffer that we read the results put into the second argument of the kernel into
	numberoutMemObj = createBuffer<int>(context, bufSize/division, kernel, commandQueue, 1);

	// This is for a local buffer for the workgroups, we only need to set the size, not the contents
	clSetKernelArg(kernel, 2, localWorkgroupSize * sizeof(int), NULL);


	// std::cout << "Enqueuing kernel with " << globalWorkSize 
	// << " global index into local group size " << localWorkgroupSize << std::endl;
	ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalWorkSize,
	                             &localWorkgroupSize, 0, NULL, &kernelEnqueueToWaitFor);
	clCheckError(ret);

	ret = clEnqueueReadBuffer(commandQueue, numberoutMemObj, CL_TRUE, 0, (bufSize/division) * sizeof(float),
	                          numbersCopy, 1, &kernelEnqueueToWaitFor, NULL);
	clCheckError(ret);
	

    // normally you clean everything up here but I haven't implented that
	

	return 0;
}

static void BM_OpenCL_Add(benchmark::State& state){
	setup(
		std::exp2(state.range(0))
		);

	for(auto _ : state){
		runAdd(std::exp2(state.range(0)), 4);
	}


	cleanContext(context);
	free(numbers);
	free(numbersCopy);

}

BENCHMARK(BM_OpenCL_Add)
	->Iterations(1000)
	->Unit(benchmark::kMillisecond)
	->DenseRange(5, 20, 1)
	// ->Arg(std::exp2(10))
	// ->Arg(std::exp2(11))
	// ->Arg(std::exp2(12))
	// ->Arg(std::exp2(13))
	// ->Arg(std::exp2(14))
	// ->Arg(std::exp2(16))
	// ->Arg(std::exp2(17))
	// ->Arg(std::exp2(18))
	// ->Arg(std::exp2(19))
	// ->Arg(std::exp2(20))
	;

int main(){
	setup(std::exp2(7));
	runAdd(std::exp2(7), 4);
	cleanContext(context);
	free(numbers);
	free(numbersCopy);

	return 0;
}
//BENCHMARK_MAIN();