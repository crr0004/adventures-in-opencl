#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include <memory>
#include <variant>

#include "ocl_template/kernel.hpp"

TEST_CASE("Run a one input and output kernel") {
	SECTION("Run a kernel") {
        auto kernel = std::make_unique<OCLT::Kernel>("TestA");
        kernel
        ->Source("kernel.cl")
        ->AddInput<float>("test")
        ->SetBuffer(std::vector<float>{32.0, 16.0, 8.0})
        ->Done()
        ;
	}
}
TEST_CASE("Creation of Context"){
        SECTION("Basic construction"){
                auto context = std::make_unique<OCLT::CLContext>();
                context
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