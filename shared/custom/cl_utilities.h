#if !defined(CL_UTILITIES)
#define CL_UTILITIES
#include "basic_headers.h"
#include <fstream>
#include <string>
#include <iostream>

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
	// fprintf(stdout, "Found %d devices.\n", num_devices);

	cl_device_id devices[num_devices];
	clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);

	for(i = 0; i < num_devices; i++) {
		printDeviceInfo(devices[i]);
	}
}
cl_context setupContext(cl_device_id *usedDeviceId) {
	// Setup the variables for context setup
	size_t platformsToGet = 5;
	cl_platform_id platform_id[platformsToGet];
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_uint deviceSelection = 0;

	// Grab the platforms
	cl_int ret = clGetPlatformIDs(platformsToGet, platform_id, &ret_num_platforms);
	//  fprintf(stdout, "%d number of platforms\n", ret_num_platforms);

	// Grab the second device
	// Change the first argument to change the platform being selected
	ret = clGetDeviceIDs(platform_id[2], CL_DEVICE_TYPE_GPU, 1, &device_id, &ret_num_devices);
	if(ret != CL_SUCCESS) {
		clCheckError(ret);
		exit(1);
	}

	// Dump query information from device
	char buf[128];
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 128, buf, NULL);
	// fprintf(stdout, "Device %s supports.\n", buf);

	for(int i = 0; i < ret_num_platforms; i++){
		// printPlatformName(platform_id[i]);
	}
	// printDeviceInfo(device_id);

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
	// fprintf(stdout, "Using %s for context device.\n", buf);

	if(usedDeviceId != nullptr){
		*usedDeviceId = used_device_id;
	}

	return context;
}



std::string readFile(const std::string& fileName, size_t *fileSize){
	std::ifstream fileStream = std::ifstream(fileName, std::ios::in);

	// Jump to the end of the file so we can know how big it is
	fileStream.seekg(0, std::ios::end);

	// See how many characters are in the file
	std::streampos size = fileStream.tellg();
	// Create an empty string to read into
	std::string programSource = std::string(size, '\0');

	// Go back to the start of the file
	fileStream.seekg(0, std::ios::beg);
	
	// Read in the file
	fileStream.read(&programSource[0], size);
	// Close of the file handle
	fileStream.close();

	*fileSize = size;
	return programSource;
}


typedef struct {
	const std::string fileName;
	const std::string entryPoint;
	const cl_context context;
	const cl_device_id deviceId;
	const std::string programOptions {"-O3"};

} CreateKernelParamaters;

cl_kernel createKernel(const CreateKernelParamaters params){

	size_t fileSize;
	std::string programSource = readFile(params.fileName, &fileSize);

	cl_int ret;
	const char* sourceStr = programSource.c_str();
	cl_program program =
	clCreateProgramWithSource(params.context, 1, (const char **)&sourceStr, (const size_t *)&fileSize, &ret);
	clCheckError(ret, "create program");

	// Build the program
    ret = clBuildProgram(program, 1, &params.deviceId, params.programOptions.c_str(), NULL, NULL);
	if(ret != CL_SUCCESS){
		clCheckError(ret, "before get build log", false);
		size_t bufSize = 1024;
		char buf[bufSize];
		size_t returnedBuf = 0;

		ret = clGetProgramBuildInfo(
			program,
			params.deviceId,
			CL_PROGRAM_BUILD_LOG,
			bufSize,
			&buf,
			&returnedBuf
		);
		clCheckError(ret, "get build log");
		if(returnedBuf == bufSize){
			std::cout << "More log entry is probably available for the program build" << "\n";
		}
		std::cout << "Log entry for last program build: " << "\n" << buf << "\n";
	}
	clCheckError(ret);
 
    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, params.entryPoint.c_str(), &ret);
	clCheckError(ret, "create kernel");

	return kernel;
}

template<typename T> cl_mem setBufferIntoKernel(
	cl_context context, 
	T *numbers, 
	size_t size,
	cl_kernel kernel, 
	cl_command_queue commandQueue,
	cl_uint index,
	cl_mem_flags memFlags = CL_MEM_READ_ONLY
	){

    cl_int ret;
	cl_mem memObject = clCreateBuffer(context, memFlags, size * sizeof(T), NULL, &ret);
	clCheckError(ret);

	ret = clEnqueueWriteBuffer(commandQueue, memObject, CL_TRUE, 0, size * sizeof(T), numbers, 0, NULL, NULL);
	clCheckError(ret);

	ret = clSetKernelArg(kernel, index, sizeof(cl_mem), (void *)&memObject);
	clCheckError(ret);

	return memObject;
}

template<typename T> cl_mem createBuffer(
	cl_context context, 
	size_t size,
	cl_kernel kernel, 
	cl_command_queue commandQueue,
	cl_uint index
){
	cl_int ret;
	cl_mem memObject =
	clCreateBuffer(context, CL_MEM_READ_WRITE, size * sizeof(T), NULL, &ret);
	clCheckError(ret, "Create local buffer");

	ret = clSetKernelArg(kernel, index, sizeof(cl_mem), (void *)&memObject);
	clCheckError(ret, "Set local buffer");

	return memObject;
}

template<typename T> cl_mem createKernelOnlyBuffer(
	cl_context context, 
	size_t size,
	cl_kernel kernel, 
	cl_command_queue commandQueue,
	cl_uint index
){
	cl_int ret;
	cl_mem memObject =
	clCreateBuffer(context, CL_MEM_READ_WRITE, size * sizeof(T), NULL, &ret);
	clCheckError(ret, "Create local buffer");

	ret = clSetKernelArg(kernel, index, sizeof(cl_mem), NULL);
	clCheckError(ret, "Set local buffer");

	return memObject;

}


#endif // CL_UTILITIS
