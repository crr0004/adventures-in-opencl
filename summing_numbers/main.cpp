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
	size_t platformsToGet = 2;
	cl_platform_id platform_id[platformsToGet];
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_uint deviceSelection = 0;

	// Grab the platforms
	cl_int ret = clGetPlatformIDs(platformsToGet, platform_id, &ret_num_platforms);
	fprintf(stdout, "%d number of platforms\n", ret_num_platforms);

	// Grab the second device
	ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_DEFAULT, deviceSelection, &device_id, &ret_num_devices);
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


cl_kernel createKernel(const char* fileName, cl_context context){

	cl_program program =
	clCreateProgramWithSource(context, 1, (const char **)&source_str, (const size_t *)&source_size, &ret);
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
}

cl_mem setBufferIntoKernel(
	cl_context context, 
	int *numbers, 
	cl_kernel kernel, 
	int index
	){

	cl_mem a_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, LIST_SIZE * sizeof(int), NULL, &ret);

	ret = clEnqueueWriteBuffer(command_queue, a_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(int), A, 0, NULL, NULL);
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
}

void executeKernel(
	cl_context context, 
	cl_kernel kernel, 
	uint32_t workGroupSize
	){
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_item_size,
	                             &local_item_size, 0, NULL, NULL)
}

void readBufferFromKernel(
	cl_context context,
	cl_kernel kernel,
	int *numbers,
	cl_mem memoryObj
){
	ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, LIST_SIZE * sizeof(int), C, 0, NULL, NULL);
}

void cleanContext(cl_context context){

}

TEST_CASE("File testing", "[test]"){

	std::ifstream a = std::ifstream("kernel.cl", std::ios::in);

	size_t size;
	std::string programSource = readFile("kernel.cl", &size);
	std::cout << "Program source" << std::endl <<
		programSource
	<< std::endl;

	REQUIRE(size > 0);

}

TEST_CASE("Device compute units"){
	cl_device_id deviceId = nullptr;
	cl_context context = setupContext(&deviceId);

	REQUIRE(deviceId != nullptr);

	cl_uint computeUnits = 0;
	size_t returnedSize = 0;
	cl_int ret = clGetDeviceInfo(
		deviceId,
		CL_DEVICE_MAX_CLOCK_FREQUENCY,
		sizeof(cl_uint),
		&computeUnits,
		&returnedSize
	);

	REQUIRE(returnedSize > 0);
	REQUIRE(computeUnits > 0);

	std::cout << "Compute units " << computeUnits << std::endl;
}

TEST_CASE("Naive Number Sum"){
	const int numberRange = 1000;
	int *numbers = new int[numberRange];
	for(int i = 0; i < numberRange; i++){
		numbers[i] = i+1;
	}

	cl_mem numbersMemObj;
	cl_kernel kernel;
	cl_device_id deviceId;
	cl_int ret;


	cl_context context = setupContext(&deviceId);
    cl_command_queue command_queue = clCreateCommandQueue(context, deviceId, 0, &ret);

	kernel = createKernel("summing_numbers.cl", context);

	numbersMemObj = setBufferIntoKernel(
		context, // context
		numbers, // buffer
		kernel, // kernel to set into
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

	executeKernel(
		context,
		kernel,
		0 // workgroup size. 0 for max
	);

	readBufferFromKernel(
		context,
		kernel,
		numbers, // array to write into
		numbersMemObj // previously created read memory object
	);


	cleanContext(context);


}

/*
int main(int argv, const char **argc) {
	cl_context context = setupContext();



    cl_int ret = clReleaseContext(context);
	clCheckError(ret);

	return 0;
}
*/