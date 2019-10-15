#include "basic_headers.h"
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_COLOUR_NONE

#include <fstream>
#include <string>
#include "catch.hpp"

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
	fprintf(stdout, "Found %d devices.\n", num_devices);

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
	fprintf(stdout, "%d number of platforms\n", ret_num_platforms);

	// Grab the second device
	ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	if(ret != CL_SUCCESS) {
		clCheckError(ret);
		exit(1);
	}

	// Dump query information from device
	char buf[128];
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 128, buf, NULL);
	fprintf(stdout, "Device %s supports.\n", buf);

	for(int i = 0; i < ret_num_platforms; i++){
		printPlatformName(platform_id[i]);
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
	fprintf(stdout, "Using %s for context device.\n", buf);

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
	clCheckError(ret);
 
    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, entryPoint, &ret);
	clCheckError(ret);

	return kernel;
}

cl_mem setBufferIntoKernel(
	cl_context context, 
	int *numbers, 
	size_t size,
	cl_kernel kernel, 
	cl_command_queue commandQueue,
	cl_uint index
	){

    cl_int ret;
	cl_mem memObject = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(int), NULL, &ret);
	clCheckError(ret);

	ret = clEnqueueWriteBuffer(commandQueue, memObject, CL_TRUE, 0, size * sizeof(int), numbers, 0, NULL, NULL);
	clCheckError(ret);

	ret = clSetKernelArg(kernel, index, sizeof(cl_mem), (void *)&memObject);
	clCheckError(ret);

	return memObject;
}

void cleanContext(cl_context context){

}

TEST_CASE("File testing", "[test]"){

	SECTION(""){
		std::ifstream a = std::ifstream("kernel.cl", std::ios::in);

		size_t size;
		std::string programSource = readFile("kernel.cl", &size);
		std::cout << "Program source" << std::endl << programSource << std::endl;

		REQUIRE(size > 0);
	}

	SECTION(""){

		cl_device_id deviceId = nullptr;
		cl_context context = setupContext(&deviceId);

		REQUIRE(deviceId != nullptr);

		cl_uint computeUnits = 0;
		size_t returnedSize = 0;
		cl_int ret = clGetDeviceInfo(deviceId, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint),
		                             &computeUnits, &returnedSize);
		clCheckError(ret);

		REQUIRE(returnedSize > 0);
		REQUIRE(computeUnits > 0);

		std::cout << "Compute units " << computeUnits << std::endl;
	}

	SECTION(""){

		const size_t numberRange = 100;
		int *numbers = new int[numberRange];
		int *numbersCopy = new int[numberRange];
		for(int i = 0; i < numberRange; i++){
			numbers[i] = i;
		}

		cl_mem numbersMemObj;
		cl_kernel kernel;
		cl_device_id deviceId;
		cl_int ret;


		cl_context context = setupContext(&deviceId);
		cl_command_queue commandQueue = clCreateCommandQueue(context, deviceId, 0, &ret);
		clCheckError(ret);

		kernel = createKernel("kernel.cl", "add", context, deviceId);

		numbersMemObj = setBufferIntoKernel(
			context, // context
			numbers, // buffer
			numberRange,
			kernel, // kernel to set into
			commandQueue,
			0 // argument index
			);

		/*
		createWriteBufferOutOfKernel(
			context,
			kernel,
			numbersSize,
			1,
			&numbersWriteMemObj
		)
		*/

		cl_event kernelEnqueueToWaitFor;	
		const size_t localWorkgroupSize = 32;
		ret = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, &numberRange, (const size_t *)&localWorkgroupSize,
									0, NULL, &kernelEnqueueToWaitFor);
		clCheckError(ret);

		ret = clEnqueueReadBuffer(commandQueue, numbersMemObj, CL_TRUE, 0, numberRange * sizeof(int), numbersCopy, 1, &kernelEnqueueToWaitFor, NULL);
		clCheckError(ret);
		for(int i = 1; i < numberRange; i++){
			numbers[i] = numbers[i] + numbers[i-1];
			REQUIRE(numbersCopy[i] == numbers[i]);
		}

		cleanContext(context);
		}

}

TEST_CASE("Device compute units", "[test]"){
}

TEST_CASE("Naive Number Sum", "[test]"){


}

/*
int main(int argv, const char **argc) {
	cl_context context = setupContext();



    cl_int ret = clReleaseContext(context);
	clCheckError(ret);

	return 0;
}
*/