#include "basic_headers.h"
#include "cl_utilities.h"
// #define CATCH_CONFIG_MAIN
// #define CATCH_CONFIG_COLOUR_NONE

#include <fstream>
#include <string>
#include <iostream>
#include <cmath>
#include <benchmark/benchmark.h>
#include <vector>
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
	size_t bufSize = numberRange; // how many results we want
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
int run(const size_t bufSize, size_t division) {
	// Tell the device that we want to run the kernel, and how it's compute space should be divided up
	const size_t localWorkgroupSize = 32; // how big each workgroup should be
	const size_t globalWorkSize = bufSize*bufSize; // total index space for the kernel

	numbersMemObj = setBufferIntoKernel<int>(context, // context
	                                    numbers, // buffer
	                                    bufSize,
	                                    kernel, // kernel to set into
	                                    commandQueue,
	                                    0 // argument index in the kernel (left to right, zero indexed)
	);
	//Create a buffer that we read the results put into the second argument of the kernel into
	numberoutMemObj = createBuffer<int>(context, bufSize*bufSize, kernel, commandQueue, 1);

	// std::cout << "Enqueuing kernel with " << globalWorkSize 
	// << " global index into local group size " << localWorkgroupSize << std::endl;
	ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalWorkSize,
	                             &localWorkgroupSize, 0, NULL, &kernelEnqueueToWaitFor);
	clCheckError(ret);

	ret = clEnqueueReadBuffer(commandQueue, numberoutMemObj, CL_TRUE, 0, (bufSize*bufSize) * sizeof(int),
	                          numbersCopy, 1, &kernelEnqueueToWaitFor, NULL);
	clCheckError(ret);

	return 0;
}

static void BM_OpenCL_Add(benchmark::State& state){
	setup(
		std::exp2(state.range(0))
		);

	for(auto _ : state){
		run(std::exp2(state.range(0)), 4);
	}


	cleanContext(context);
	free(numbers);
	free(numbersCopy);

}

/**
 * Generates permutations according to lexicographic generation
 * using the algorithm from Algorithm L in Chapter 7.2.1.3 of 
 * The Art of Computer Programming Volume 4A Part 1 by Donald E. Knuth.
 * 
 * param n the size of the index can take in range {0...n-1}
 * param t how many positions you want in the permutation
 * returns a contingious vector of the permutations
 **/
static std::vector<uint32_t> Generate_Permutations(const uint32_t n, const uint32_t t){
	// Complete list
	std::vector<uint32_t> sup_c;

	// Temp c to keep list in, +2 for the sentenials
	uint32_t* c = new uint32_t[t+2];

	for(int i = 0; i < t; i++){
		c[i] = i;
		sup_c.push_back(i);
	}

	// Put some sentenials in
	c[t] = n;
	c[t+1] = 0;

	uint32_t j = 0;
	while(true){
		j = 0;
		// Keep going until we find the largest index we can increase
		while(c[j]+1 == c[j+1]){
			c[j] = j;
			j++;
		}

		// If we've gone too far to hit the sentenials we stop the algorthm
		if((j+1) > t){
			break;
		}

		// Increase the largest index we can
		c[j] = c[j]+1;

		// We could probably store the indexes as we can increase
		// but we will just do it here instead for simplicity
		for(int i = 0; i < t; i++){
			sup_c.push_back(c[i]);
		}
	}

	sup_c.shrink_to_fit();
	return sup_c;
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
	// setup(10);
	// run(10, 2);
	// cleanContext(context);
	// free(numbers);
	// free(numbersCopy);
	int t = 3;
	std::vector<uint32_t> numbers = Generate_Permutations(4, t);

	for(int i = 0; i < numbers.size(); i++){
		std::cout << numbers[i] << ", ";
		if((i+1) % t == 0){
			std::cout << std::endl;
		}
	}


	return 0;
}
//BENCHMARK_MAIN();

