#include "basic_headers.h"
#include "cl_utilities.h"
// #define CATCH_CONFIG_MAIN
// #define CATCH_CONFIG_COLOUR_NONE

#include <fstream>
#include <string>
#include <iostream>
#include <cmath>
#include <benchmark/benchmark.h>
// #include "catch.hpp"


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