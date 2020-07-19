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
#include <climits>
#include <tuple>
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
std::vector<float> numbers;
std::vector<int> numbersCopy;

// prevent compiler from optimising CPU add
float numOutResult = 0;

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

	commandQueue = nullptr;
	kernelEnqueueToWaitFor = nullptr;
	program = nullptr;
	kernel = nullptr;
	numberoutMemObj = nullptr;
	numbersMemObj = nullptr;
	deviceId = nullptr;
	context = nullptr;
}

void setupNumbers(const size_t numberRange){
	// From https://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
	std::random_device rd; // Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(1, 9);
 
	numbers = std::vector<float>(numberRange);

	// create the buffer we are going to write the results into
	size_t bufSize = numberRange; // how many results we want
	// numbersCopy = std::vector<int>(bufSize);

	// std::cout << "Summing: ";
	// numbers = {6, 7, 4, 9}; // uncomment for hard coding, comment out loop as well

	// fill in the numbers between our range we set in dis()
	for(int i = 0; i < numberRange; i++) {
		numbers[i] = (float)1;
		// std::cout << numbers[i] << ", ";
	}

}
/**
 * @brief Setup the random numbers and buffers for the kernels to run on.
 * @see run
 * 
 * @param numberRange how many numbers to generate between 1-9
 * @param dividers how many dividers you want to solve for I.E dividers+1 is how many painters there are
 */
std::tuple<size_t, size_t> setup(const size_t numberRange, const std::string filePath){
	setupNumbers(numberRange);

	// Move the number range back one for permutations because we are doing exclusive range
	// partitionsPermutations = Generate_Permutations<uint32_t>(numberRange-1, dividers);
	
	/*
	// Dump out the permutations
	for(int i = 0; i < partitionsPermutations.size(); i++){
		std::cout << partitionsPermutations[i] << ", ";
		if((i+1) % partitions == 0){
			// std::cout << "\n";
		}
	}
	*/
	

	// Create the buffer we are going to operate on
//	const size_t numberRange = 4*std::exp2(25);
	assert(!filePath.empty());
	// std::cout << "\;

	// Create and setup the context to and assign the deviceId we will be using
	context = setupContext(&deviceId);

	// Create a command queue to push commands into
	commandQueue = clCreateCommandQueue(context, deviceId, 0, &ret);
	clCheckError(ret, "create command queue");

	// Create our compute kernel for the device, context and the entry point
	kernel = createKernel(
		{
		filePath,
		"add", // entry point name
		context, // opencl context
		deviceId, // device to build against
		"-O3 -cl-std=CL2.0"
		}
		);

	numbersMemObj = setBufferIntoKernel<float>(context, // context
	                                    numbers.data(), // buffer
	                                    numbers.size(),
	                                    kernel, // kernel to set into
	                                    commandQueue,
	                                    0 // argument index in the kernel (left to right, zero indexed)
	);
	// sets some constants
	size_t numbersSize = numbers.size();
	ret = clSetKernelArg(kernel, 1, sizeof(int), (const void*)&numbersSize);
	clCheckError(ret, "set numbersSize");

	const size_t localGroupSize = 64;

	// Set the default out number to the min function works correctly
	float numOutDefault {0};
	// Create a buffer that we read the results put into
	numberoutMemObj = setBufferIntoKernel<float>(
		context,
		&numOutDefault,
		1,
		kernel,
		commandQueue,
		2,
		CL_MEM_READ_WRITE
	);

	createKernelOnlyBuffer<float>(
		context,
		localGroupSize,
		kernel,
		commandQueue,
		3
	);

	return {localGroupSize, numbersSize/localGroupSize};
}

/**
 * @brief Runs the kernel for solving the painters partition problem.
 * 
 * @param dividers how many dividers you want to solve for I.E dividers+1 is how many painters there are
 * @return int the optimum partition size
 */
int run(const std::tuple<size_t, size_t>& sizes) {
	// Tell the device that we want to run the kernel, and how it's compute space should be divided up
	const size_t localWorkgroupSize {std::get<0>(sizes)}; // how big each workgroup should be
	const size_t globalWorkSize {std::get<1>(sizes)};
	

	// std::cout << "Enqueuing kernel with " << globalWorkSize 
	// << " global index into local group size " << localWorkgroupSize << "\;
	ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &globalWorkSize,
	                             &localWorkgroupSize, 0, NULL, &kernelEnqueueToWaitFor);
	clCheckError(ret, "enqueue kernel");

	// Read out the write buffer we set before
	float numOut { (float)INT_MAX };
	ret = clEnqueueReadBuffer(commandQueue, numberoutMemObj, CL_TRUE, 0, sizeof(float),
	                           (void *)&numOut, 1, &kernelEnqueueToWaitFor, NULL);
	clCheckError(ret, "read buffer");
	numOutResult = numOut;

	return 0;
}

static int run_cpu_add(){
	// std::cout << "CPU add: " <<
		return std::accumulate(begin(numbers), end(numbers), 0) ;
	// << "\n";
}

/**
 * @brief run the benchmark
 * 
 * @param state given by the benchmark framework
 */
static void BM_OpenCL_Add(benchmark::State& state){
	auto sizes = setup(
		64*state.range(0), "kernel.cl");

	for(auto _ : state){
		run(sizes);
	}


	cleanContext(context);
}
static void BM_setup_deconstruct(benchmark::State& state){
	auto sizes = setup(
		64*state.range(0), "kernel.cl");
		run(sizes);

	cleanContext(context);

}
static void BM_CPU_Add(benchmark::State& state){

	setupNumbers(64*state.range(0));
	for(auto _ : state){
		numOutResult = run_cpu_add();
	}

}

// BENCHMARK(BM_setup_deconstruct)
// 	->Iterations(100)
// 	->Unit(benchmark::kMillisecond)
// 	->Arg(64)
// 	;

BENCHMARK(BM_OpenCL_Add)
	->Iterations(100000)
	// ->Iterations(1)
	->Unit(benchmark::kMillisecond)
// 	->DenseRange(19, 22, 1)
	->Arg(64)
	;
// // uncomment to run benchmark, remember to comment out int main()
BENCHMARK(BM_CPU_Add)
// 	// ->Iterations(100)
	->Unit(benchmark::kMillisecond)
// 	->DenseRange(19, 22, 1)
	->Arg(64)
// 	// ->Arg(20)
	;

BENCHMARK_MAIN();



// int main(int argc, char* argv[]){
// 	int numbersToGenerate = 64*64;
// 	if(argc > 1){
// 		auto sizes = setup(numbersToGenerate, std::string(argv[1]));
// 		run(sizes);
// 	}else{
// 		auto sizes = setup(numbersToGenerate, "kernel.cl");
// 		run(sizes);
// 	}
// 	std::cout << "Sum " << numOutResult << "\n";
// 	cleanContext(context);
// 	// free(numbers);
// 	// free(numbersCopy);



// 	return 0;
// }
