#include <catch.hpp>
#include <memory>
#include <iostream>

#include "ocl_template/kernel.hpp"

CATCH_TRANSLATE_EXCEPTION(OCLT::KernelNotFound& ex){
	return ex.what();
}

TEST_CASE("Kernel creation") {
	SECTION("Run a kernel") {
				auto contextBuilder = std::make_unique<OCLT::CLContext>();
				auto context = contextBuilder
					->SetType(CL_DEVICE_TYPE_GPU)
					->SetPreferedPlatformIndex(0)
					->SetPreferedDeviceIndex(0)
					->Find();
				std::cout << "Device name and info\t";
				context->StreamNameAndVersionInto(std::cout);
				std::cout << "\n";

        auto kernel = std::make_unique<OCLT::Kernel>("TestA");
        auto builtKernel = kernel
				->EntryPoint("test")
        ->Source("kernel.cl")
				->Build(context.get());
			//	->Create(context)
        OCLT::Stateless::DumpProgramInfo(builtKernel->program);
        ;
	}
	SECTION("Run a kernel with defaults"){
				auto contextBuilder = std::make_unique<OCLT::CLContext>();
				auto context = contextBuilder
					->SetType(CL_DEVICE_TYPE_GPU)
					->SetPreferedPlatformIndex(0)
					->SetPreferedDeviceIndex(0)
					->Find();

        auto kernel = std::make_unique<OCLT::Kernel>("TestA");
				auto builtKernel = kernel->Build(context.get());

	}
}
TEST_CASE("Buffers"){
	auto contextBuilder = std::make_unique<OCLT::CLContext>();
	auto context = contextBuilder
		->SetType(CL_DEVICE_TYPE_GPU)
		->SetPreferedPlatformIndex(0)
		->SetPreferedDeviceIndex(0)
		->Find();
	SECTION("Create an input buffer"){
		auto sampleData = std::vector{2.0f, 4.0f, 8.0f, 16.0f};
		auto buffer = OCLT::BufferBuilder<float>()
			.Size(sampleData.size())
			->Build(context.get());

		REQUIRE(buffer->size == sampleData.size());
	}
	SECTION("Write and read to input are equal"){
		auto sampleData = std::vector{2.0f, 4.0f, 8.0f, 16.0f};
		auto buffer = OCLT::BufferBuilder<float>()
			.Size(sampleData.size())
			->Build(context.get());

		REQUIRE(buffer->size == sampleData.size());

		auto options = OCLT::BufferOptions{};
		options.block = CL_TRUE;

		buffer->Write(context.get(), sampleData.data(), options);

		auto newData = std::vector<float>(sampleData.size());
		buffer->Read(context.get(), newData.data(), options);

		REQUIRE(newData == sampleData);

	}
}
TEST_CASE("Creation of Context"){
        SECTION("Basic construction"){
                auto context = std::make_unique<OCLT::CLContext>();
                auto built = context
                ->SetType(CL_DEVICE_TYPE_GPU)
                ->SetPreferedPlatformIndex(0)
                ->SetPreferedDeviceIndex(0)
                ->Find();

        }
        SECTION("Can print platform information"){
                auto context = std::make_unique<OCLT::CLContext>();
                context->PrintPlatforms(std::cout);
	    }
}

