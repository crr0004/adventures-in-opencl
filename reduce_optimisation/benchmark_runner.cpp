#include <benchmark/benchmark.h>
#include <numeric>
#include <cmath>
#include "ocl_template/kernel.hpp"

static void BM_OpenCL_Add(benchmark::State& state){
	auto contextBuilder = std::make_unique<OCLT::CLContext>();

	auto context = contextBuilder
		->SetType(CL_DEVICE_TYPE_GPU)
		->SetPreferedPlatformIndex(2)
		->SetPreferedDeviceIndex(0)
		->Find();

	auto kernel = std::make_unique<OCLT::Kernel>("Sum")
		->EntryPoint("add")
		->Source("kernel.cl")
		->Build(context.get());

	auto numbers = std::vector<float>(64*state.range(0), 1);

	// Create buffer options that will block on buffer IO
	OCLT::BufferOptions blockIO;
	blockIO.block = CL_TRUE;

	auto input = OCLT::BufferBuilder<float>()
		.Size(numbers.size())
		->Build(context.get());
	auto writeBlock = input->Write(context.get(), numbers.data(), blockIO);

	auto numbersSizeInput = OCLT::BufferBuilder<int>()
		.Size(1)
		->Flags(CL_MEM_READ_ONLY)
		->Build(context.get());
	int size = numbers.size();

	numbersSizeInput->Write(context.get(), &size, blockIO);

	auto output = OCLT::BufferBuilder<float>()
		.Size(1)
		->Build(context.get());


	input->SetAsKernelArg(kernel.get(), 0);
	kernel->SetLiteralKernelArg<int>(numbers.size(), 1);
	output->SetAsKernelArg(kernel.get(), 2);
	//kernel->SetLocalKernelArg<float>(64, 3);

	for(auto _ : state){

		const size_t localWorkGroupSize = 64;
		auto event = kernel->Run(context.get(),
				{numbers.size()/localWorkGroupSize, localWorkGroupSize, 1, 0, 0, nullptr}
				);

		clWaitForEvents(1, &event);
		//clFinish(context->commandQueue);

		auto resultVector = std::vector{0.0f};
		output->Read(context.get(), resultVector.data(), blockIO);
		//std::cout << resultVector[0] << "\n";
		benchmark::ClobberMemory();

	}
}

static void BM_OpenCL_Add_No_Global_Mem_Read(benchmark::State& state){
	auto contextBuilder = std::make_unique<OCLT::CLContext>();

	auto context = contextBuilder
		->SetType(CL_DEVICE_TYPE_GPU)
		->SetPreferedPlatformIndex(2)
		->SetPreferedDeviceIndex(0)
		->Find();

	auto kernel = std::make_unique<OCLT::Kernel>("Sum")
		->EntryPoint("add")
		->Source("looped_sum_kernel.cl")
		->Build(context.get());

	auto numbers = std::vector<float>(64*state.range(0), 1);

	// Create buffer options that will block on buffer IO
	OCLT::BufferOptions blockIO;
	blockIO.block = CL_TRUE;

	auto input = OCLT::BufferBuilder<float>()
		.Size(numbers.size())
		->Build(context.get());
	auto writeBlock = input->Write(context.get(), numbers.data(), blockIO);

	auto numbersSizeInput = OCLT::BufferBuilder<int>()
		.Size(1)
		->Flags(CL_MEM_READ_ONLY)
		->Build(context.get());
	int size = numbers.size();

	numbersSizeInput->Write(context.get(), &size, blockIO);

	auto output = OCLT::BufferBuilder<float>()
		.Size(1)
		->Build(context.get());


	input->SetAsKernelArg(kernel.get(), 0);
	kernel->SetLiteralKernelArg<int>(numbers.size(), 1);
	output->SetAsKernelArg(kernel.get(), 2);
	//kernel->SetLocalKernelArg<float>(64, 3);

	for(auto _ : state){

		const size_t localWorkGroupSize = 64;
		auto event = kernel->Run(context.get(),
				{numbers.size()/localWorkGroupSize, localWorkGroupSize, 1, 0, 0, nullptr}
				);

		clWaitForEvents(1, &event);
		//clFinish(context->commandQueue);

		//	auto resultVector = std::vector{0.0f};
		//	output->Read(context.get(), resultVector.data(), blockIO);
		//std::cout << resultVector[0] << "\n";
		benchmark::ClobberMemory();

	}
}
static void BM_OpenCL_OpenDevice(benchmark::State& state){

	for(auto _ : state){
		auto contextBuilder = std::make_unique<OCLT::CLContext>();

		auto context = contextBuilder
			->SetType(CL_DEVICE_TYPE_GPU)
			->SetPreferedPlatformIndex(2)
			->SetPreferedDeviceIndex(0)
			->Find();
	}
}
static void BM_OpenCL_Create_Destroy(benchmark::State& state){

	for(auto _ : state){
		auto contextBuilder = std::make_unique<OCLT::CLContext>();

		auto context = contextBuilder
			->SetType(CL_DEVICE_TYPE_GPU)
			->SetPreferedPlatformIndex(2)
			->SetPreferedDeviceIndex(0)
			->Find();
		auto kernel = std::make_unique<OCLT::Kernel>("Sum")
			->EntryPoint("add")
			->Source("kernel.cl")
			->Build(context.get());
	}

}
static void BM_Accumulate(benchmark::State& state){
	auto numbers = std::vector<float>(64*state.range(0), 1);
	for(auto _ : state){
		benchmark::DoNotOptimize(std::accumulate(numbers.begin(), numbers.end(), 0));
	}
}

static void Kernel_Accumulate(benchmark::State& state){

	auto contextBuilder = std::make_unique<OCLT::CLContext>();

	auto context = contextBuilder
		->SetType(CL_DEVICE_TYPE_GPU)
		->SetPreferedPlatformIndex(2)
		->SetPreferedDeviceIndex(0)
		->Find();

	auto sum_kernel = std::make_unique<OCLT::Kernel>("Sum")
		->EntryPoint("add")
		->Source("base_sum_kernel.cl")
		->Build(context.get());

	auto add_numbers_kernel = std::make_unique<OCLT::Kernel>("add_numbers")
		->EntryPoint("add")
		->Source("add_numbers_kernel.cl")
		->Build(context.get());

	auto numbers = std::vector<float>(64*state.range(0), 1);

	// Create buffer options that will block on buffer IO
	OCLT::BufferOptions blockIO;
	blockIO.block = CL_TRUE;

	auto input = OCLT::BufferBuilder<float>()
		.Size(numbers.size())
		->Build(context.get());
	auto writeBlock = input->Write(context.get(), numbers.data(), blockIO);

	size_t numbers_handled = std::ceil((float)numbers.size()/(64*64)); // 64*64 is the max amount of numbers that can be handled
	auto output = OCLT::BufferBuilder<float>()
		.Size(numbers_handled) 
		->Build(context.get());


	input->SetAsKernelArg(sum_kernel.get(), 0);
	sum_kernel->SetLiteralKernelArg<int>(numbers.size(), 1);
	output->SetAsKernelArg(sum_kernel.get(), 2);
	//kernel->SetLocalKernelArg<float>(64, 3);
	output->SetAsKernelArg(add_numbers_kernel.get(), 0);
	add_numbers_kernel->SetLiteralKernelArg<int>((int)numbers_handled, 1);

	for(auto _ : state){

		const size_t localWorkGroupSize = 64;
		auto event = sum_kernel->Run(context.get(), { numbers.size() / localWorkGroupSize,
				localWorkGroupSize, 1, 0, 0, nullptr });

		clWaitForEvents(1, &event);
		auto final_event = add_numbers_kernel->Run(context.get(), {1, 1, 1, 0, 0, nullptr});
		clWaitForEvents(1, &final_event);

		
		/*
		auto resultVector = std::vector{0.0f, 0.0f};
		output->Read(context.get(), resultVector.data(), blockIO, 2);
		std::cout << resultVector[0] << "\n";
		*/
		benchmark::ClobberMemory();

	}
}

BENCHMARK(Kernel_Accumulate)
->Unit(benchmark::kMillisecond)
	->Iterations(10000)
	->DenseRange(65,128,1)
	;
BENCHMARK(BM_OpenCL_Add_No_Global_Mem_Read)
	->Unit(benchmark::kMillisecond)
	->Iterations(10000)
	->DenseRange(50, 64, 1)
	;
BENCHMARK(BM_OpenCL_Add)
	->Unit(benchmark::kMillisecond)
	->Iterations(10000)
	->DenseRange(50, 64, 1)
	//->Arg(64)
	;
BENCHMARK(BM_Accumulate)
	->Unit(benchmark::kMillisecond)
	->Iterations(10000)
	->DenseRange(50, 64, 1)
	//->Arg(64)
	;

BENCHMARK(BM_OpenCL_OpenDevice)
	->Unit(benchmark::kMillisecond);

BENCHMARK(BM_OpenCL_Create_Destroy)
	->Unit(benchmark::kMillisecond);

	BENCHMARK_MAIN();
