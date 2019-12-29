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
#include <random>
// #include "catch.hpp"


// some openCL API objects
cl_context context;
cl_mem numbersMemObj; // buffer memory pointer
cl_kernel kernel; // kernel pointer
cl_device_id deviceId; // device we are going to use
cl_command_queue commandQueue;
cl_mem numberoutMemObj;
cl_mem partitionsMemObj;
cl_int ret; // error number holder
cl_event kernelEnqueueToWaitFor; // a sync event to wait for
std::vector<int> numbers;
std::vector<int> numbersCopy;
std::vector<uint32_t> partitionsPermutations;

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

int setup(const size_t numberRange, const size_t partitions){

	// Move the number range back one for permutations because we are doing exclusive range
	partitionsPermutations = Generate_Permutations<uint32_t>(numberRange-1, partitions);
	
	/*
	// Dump out the permutations
	for(int i = 0; i < partitionsPermutations.size(); i++){
		std::cout << partitionsPermutations[i] << ", ";
		if((i+1) % partitions == 0){
			// std::cout << std::endl;
		}
	}
	*/
	

	// Create the buffer we are going to operate on
//	const size_t numberRange = 4*std::exp2(25);
	// From https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
	std::random_device rd; // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(1, 9);
 
	numbers = std::vector<int>(numberRange);

	// create the buffer we are going to write the results into
	size_t bufSize = numberRange; // how many results we want
	numbersCopy = std::vector<int>(bufSize);

	// fill in the numbers between our range we set in dis()
	// std::cout << "Summing: ";
	// numbers = {6, 7, 4, 9};
	for(int i = 0; i < numberRange; i++) {
		numbers[i] = dis(gen);
		// std::cout << numbers[i] << ", ";
	}
	std::cout << std::endl;

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

int run(const size_t partitions) {
	// Tell the device that we want to run the kernel, and how it's compute space should be divided up
	const size_t localWorkgroupSize = 32; // how big each workgroup should be
	const size_t globalWorkSize = partitionsPermutations.size()/partitions;

	partitionsMemObj = setBufferIntoKernel<uint32_t>(context, // context
	                                    partitionsPermutations.data(), // buffer
	                                    partitionsPermutations.size(),
	                                    kernel, // kernel to set into
	                                    commandQueue,
	                                    0 // argument index in the kernel (left to right, zero indexed)
	);

	numbersMemObj = setBufferIntoKernel<int>(context, // context
	                                    numbers.data(), // buffer
	                                    numbers.size(),
	                                    kernel, // kernel to set into
	                                    commandQueue,
	                                    1 // argument index in the kernel (left to right, zero indexed)
	);
	ret = clSetKernelArg(kernel, 2, sizeof(int), (const void*)&partitions);
	clCheckError(ret);

	size_t numbersSize = numbers.size();
	ret = clSetKernelArg(kernel, 3, sizeof(int), (const void*)&numbersSize);
	clCheckError(ret);

	// allocate a local buffer for the workgroups
	ret = clSetKernelArg(kernel, 4, sizeof(int) * localWorkgroupSize, NULL);
	clCheckError(ret);

	// Create a buffer that we read the results put into
	numberoutMemObj = createBuffer<int>(context, 1, kernel, commandQueue, 5);

	// std::cout << "Enqueuing kernel with " << globalWorkSize 
	// << " global index into local group size " << localWorkgroupSize << std::endl;
	ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalWorkSize,
	                             &localWorkgroupSize, 0, NULL, &kernelEnqueueToWaitFor);
	clCheckError(ret);

	// Read out the write buffer we set before
	int numOut = -1;
	ret = clEnqueueReadBuffer(commandQueue, numberoutMemObj, CL_TRUE, 0, sizeof(int),
	                           (void *)&numOut, 1, &kernelEnqueueToWaitFor, NULL);
	clCheckError(ret);
	std::cout << "Max " << numOut << std::endl;

	return 0;
}

static void BM_OpenCL_Add(benchmark::State& state){
	size_t partitions = 3;
	setup(
		std::exp2(state.range(0)),
		partitions
		);

	for(auto _ : state){
		run(partitions);
	}


	cleanContext(context);
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
	int partitions = 2;
	setup(400, partitions);
	run(partitions);
	cleanContext(context);
	// free(numbers);
	// free(numbersCopy);



	return 0;
}
//BENCHMARK_MAIN();

