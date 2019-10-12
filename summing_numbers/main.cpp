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
cl_context setupContext() {
	// Setup the variables for context setup
	cl_platform_id platform_id[2];
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_uint deviceSelection = 1;

	// Grab the platforms
	cl_int ret = clGetPlatformIDs(2, platform_id, &ret_num_platforms);

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

	printPlatformName(platform_id[0]);

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

cl_uint createKernel(const char* fileName){


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



/*
int main(int argv, const char **argc) {
	cl_context context = setupContext();



    cl_int ret = clReleaseContext(context);
	clCheckError(ret);

	return 0;
}
*/