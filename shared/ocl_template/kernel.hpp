#pragma once

#include <CL/cl.h>
#include <string>
#include <vector>
#include "ocl_template/exceptions.hpp" 
#include <memory>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "custom/clext.h"

namespace OCLT{
	class Kernel;
	class BuiltCLContext;

	namespace Stateless{
		static const std::vector<cl_platform_id> GetPlatforms();
		static const std::string ReadFile(const std::string &fileName);
		static const void PrintBuildLog(cl_int ret, cl_program program, cl_device_id device);
		static const void DumpProgramInfo(cl_program);
		template<typename T> static void PrintPlatforms(T& out);

	} // namespace Stateless

	class BuiltCLContext {
		public:
			friend class CLContext;
			friend class Kernel;
			const cl_context context;
			const cl_command_queue commandQueue;
			const cl_device_id device;
			~BuiltCLContext() {
				clFlush(commandQueue);
				clFinish(commandQueue);
				clReleaseCommandQueue(commandQueue);

				clReleaseDevice(device);
				clReleaseContext(context);
			}
			cl_context getContext(){
				return context;
			}
			template<typename T>
				void StreamNameAndVersionInto(T& out){

					std::vector<char> buf;
					size_t bufSize;

					clGetDeviceInfo(device, CL_DEVICE_NAME, 0, nullptr, &bufSize);
					buf.reserve(bufSize);
					clGetDeviceInfo(device, CL_DEVICE_NAME, bufSize, buf.data(), NULL);

					out << buf.data();


					buf.clear();
					clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, nullptr, &bufSize);
					buf.reserve(bufSize);
					clGetDeviceInfo(device, CL_DEVICE_VERSION, bufSize, buf.data(), NULL);

					out << buf.data();

				}
		private:	
			BuiltCLContext(cl_context context, cl_command_queue commandQueue, cl_device_id device)
				: context(context), commandQueue(commandQueue), device(device) {}
	};
	class CLContext {
		public:
			CLContext(){}
			~CLContext(){}

			CLContext* SetType(cl_device_type type){
				this->type = type;
				return this;
			}
			CLContext* SetPreferedDeviceIndex(int index){
				this->deviceIndex = index;
				return this;
			}
			CLContext* SetPreferedPlatformIndex(int index){
				this->platformIndex = index;
				return this;
			}

			template<typename T>
				void PrintPlatforms(T& out){
					auto platforms = Stateless::GetPlatforms();

					size_t outBufSize = 0;
					int index = 0;
					for(auto platformId : platforms) {
						auto ret = clGetPlatformInfo(platformId, CL_PLATFORM_NAME, 0, nullptr, &outBufSize);
						clCheckError(ret, "Get Platform Info Size", false);

						std::vector<char> outBuf(outBufSize);
						ret = clGetPlatformInfo(platformId, CL_PLATFORM_NAME, outBufSize, outBuf.data(), nullptr);
						clCheckError(ret, "Get Platform Info", false);
						out << index << ".Platform name: " << outBuf.data() << "\n";
						index++;
					}
				}
			[[nodiscard]] std::unique_ptr<BuiltCLContext> Find() {
				auto platforms = Stateless::GetPlatforms();
				if(platforms.empty()){
					throw NoPlatformsFound();
				}

				if(platformIndex >= platforms.size()) {
			        PrintPlatforms(std::cout);
			        throw PlatformIndexOutOfRange(platformIndex, platforms.size());
				}

				auto devices = getDevicesForPlatform(platforms[platformIndex]);
				if(devices.empty()){
					throw NoDevicesFound();
				}

				auto device = devices[deviceIndex];
				this->platform = platforms[platformIndex];

				cl_int ret;
				cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
				clCheckError(ret, "Create context", false);
				// Methods are functionaly the same, just the later has a deprecation warning
#ifdef CL_VERSION_2_0
				auto commandQueue = clCreateCommandQueueWithProperties(context, device, nullptr, &ret);
#else
				auto commandQueue = clCreateCommandQueue(context, device, 0, &ret);
#endif
				clCheckError(ret, "create command queue", false);
				// Manually create a pointer because we have the BuiltCLContext
				// as private and we don't want to set std::unique_ptr as a friend
				// class
				auto builtContext = new BuiltCLContext{context, commandQueue, device};
				return std::unique_ptr<BuiltCLContext>{builtContext};
			}
			const std::vector<cl_device_id> getDevicesForPlatform(const cl_platform_id platform) {
				cl_uint numDevices = 0;
				auto ret = clGetDeviceIDs(platform, type, 0, nullptr, &numDevices);
				clCheckError(ret, "Get device ids", false);

				std::vector<cl_device_id> devices(numDevices);
				ret = clGetDeviceIDs(platform, type, numDevices, devices.data(), &numDevices);
				clCheckError(ret, "Get device ids", false);

				return devices;
			}


		private:
			cl_device_type type;
			cl_platform_id platform;
			int deviceIndex = 0;
			int platformIndex = 0;

	};
	typedef struct RunOptions{
		size_t globalWorkSize;
		size_t localWorkSize;
		cl_uint workDim = 1;
		size_t globalWorkOffset = 0;
		cl_uint numEventsToWaitFor = 0;
		cl_event* eventWaitList = nullptr;
	} RunOptions;
	class BuiltKernel{
		public:
			friend class Kernel;
			const cl_kernel kernel;
			const cl_program program;
			template<typename T> cl_int SetLiteralKernelArg(T literal, cl_int index){
					cl_int ret = clSetKernelArg(kernel, index, sizeof(T), (const void*)&literal);

					clCheckError(ret, "set kernel arg", false);
					return ret;

			}
			template<typename T> cl_int SetLocalKernelArg(size_t count, cl_int index){
				cl_int ret = clSetKernelArg(kernel, index, count*sizeof(T), nullptr);

				clCheckError(ret, "set kernel arg", false);
				return ret;
			}

			cl_event Run(const BuiltCLContext* context, const RunOptions& options){
				cl_event event;
				cl_int ret = clEnqueueNDRangeKernel(
						context->commandQueue,
						kernel,
						options.workDim,
						(const size_t*)&options.globalWorkOffset,
						(const size_t*)&options.globalWorkSize,
						(const size_t*)&options.localWorkSize,
						options.numEventsToWaitFor,
						(const cl_event*)options.eventWaitList,
						&event
						);
				clCheckError(ret, "run kernel", false);


				return event;

			}
			~BuiltKernel(){
				clReleaseKernel(kernel);
				clReleaseProgram(program);
			};
		private:
			BuiltKernel(cl_kernel kernel, cl_program program) : kernel(kernel), program(program){}

	};

	class Kernel {
		public:
			typedef std::unique_ptr<Kernel> Ptr;
			Kernel(std::string name): name(name){}
			Kernel* Source(std::string source){
				this->source = source;
				return this;
			};
			Kernel* EntryPoint(std::string entryPoint){
				this->entryPoint = entryPoint;
				return this;
			};

			[[nodiscard]] std::unique_ptr<BuiltKernel> Build(const BuiltCLContext* context) {
				auto programSource = Stateless::ReadFile(source);
				cl_int ret;
				// Need to take addresses below so need to capture beyond r-value
				auto source = programSource.c_str();
				//auto sourceSize = programSource.size();

				auto program = clCreateProgramWithSource(
						context->context,
						1,
						(const char**)&source,
						NULL,
						&ret);

				clCheckError(ret, "create program", false);

				ret = clBuildProgram(program, 1, &context->device, programOptions.c_str(), NULL, NULL);
				Stateless::PrintBuildLog(ret, program, context->device);
				clCheckError(ret, "build program", false);

				cl_kernel kernel = clCreateKernel(program, entryPoint.c_str(), &ret);
				clCheckError(ret, "create kernel", false);

				// again do this because of private constructors
				auto builtKernel = new BuiltKernel{kernel, program};
				return std::unique_ptr<BuiltKernel>{builtKernel};
			}

		private:
			std::string name = "default_name";
			std::string source = "kernel.cl";
			std::string entryPoint = "test";
			std::string programOptions = "-O3";
	};

	typedef struct BufferOptions{
		cl_bool block = CL_FALSE;
		size_t offset = 0;
		cl_uint numEventsInList = 0;
		cl_event* eventList = nullptr;
	} BufferOptions;

	template<typename T>
		class BuiltBuffer{
			public:
				BuiltBuffer(cl_mem memObject, size_t size) : memObject(memObject), size(size){
					if(size < 0){
						throw BufferSizeMustBeGreaterThanZero();
					}
				}
				cl_event Write(const BuiltCLContext* context, const T* pointer){
					Write(context, pointer, BufferOptions{});
				}
				cl_event Write(const BuiltCLContext* context, const T* pointer, const BufferOptions& options){
					cl_event event;
					cl_int ret = clEnqueueWriteBuffer(
							context->commandQueue,
							memObject,
							options.block,
							options.offset,
							size*sizeof(T),
							pointer,
							options.numEventsInList,
							options.eventList,
							&event
							);
					clCheckError(ret, "write buffer", false);

					return event;
				};
				cl_event Read(const BuiltCLContext* context, T* pointer, const BufferOptions options){
          return this->Read(context, pointer, options, this->size);
				}
        cl_event Read(const BuiltCLContext* context, T* pointer, const BufferOptions options, size_t size_override){
					cl_event event;
					cl_int ret = clEnqueueReadBuffer(
							context->commandQueue,
							memObject,
							options.block,
							options.offset,
							size_override*sizeof(T),
							pointer,
							options.numEventsInList,
							options.eventList,
							&event
							);
					clCheckError(ret, "read buffer", false);

					return event;

        }
				cl_int SetAsKernelArg(BuiltKernel* kernel, cl_int index){
					cl_int ret = clSetKernelArg(kernel->kernel, index, sizeof(cl_mem), (void *)&memObject);

					clCheckError(ret, "set kernel arg", false);
					return ret;
				}
				template<typename O> void DumpMemObjectInfo(O& out){
					size_t memSize;
					auto ret = clGetMemObjectInfo(memObject, CL_MEM_SIZE, sizeof(memSize), &memSize, nullptr);
					out << "Memsize " << memSize;

					out << "\n";
				};
				const cl_mem memObject;
				const size_t size;
				~BuiltBuffer(){
					clReleaseMemObject(memObject);
				}
		};
	template<typename T>
		class BufferBuilder{
			public:
				BufferBuilder* Size(size_t size){
					this->size = size;
					return this;
				};
				BufferBuilder* Flags(cl_mem_flags flags){
					this->flags = flags;
					return this;
				}

				std::unique_ptr<BuiltBuffer<T>> Build(const BuiltCLContext* context){
					if(size <= 0){
						throw BufferSizeMustBeGreaterThanZero();
					}
					cl_int ret;
					cl_mem memObject =
						clCreateBuffer(context->context, flags, size * sizeof(T), nullptr, &ret);
					clCheckError(ret, "create buffer", false);

					return std::make_unique<BuiltBuffer<T>>(memObject, size);
				}

			private:
				size_t size = 0;
				cl_mem_flags flags = CL_MEM_READ_WRITE;
		};
	namespace Stateless{
		const std::vector<cl_platform_id> GetPlatforms() {
			cl_uint numPlatforms = 0;
			cl_int ret = clGetPlatformIDs(0, nullptr, &numPlatforms);
			clCheckError(ret, "Get PlatformID", false);

			std::vector<cl_platform_id> platforms(numPlatforms);
			ret = clGetPlatformIDs(numPlatforms, platforms.data(), NULL);
			clCheckError(ret, "Get PlatformID", false);

			return platforms;
		}

		const void DumpProgramInfo(cl_program program) {
			size_t bufSize = 0;
			clGetProgramInfo(program, CL_PROGRAM_SOURCE, 0, nullptr, &bufSize);

			std::vector<char> buf(bufSize);
			clGetProgramInfo(program, CL_PROGRAM_SOURCE, bufSize, buf.data(), &bufSize);
			std::cout << "Program source " << buf.data() << "\n";
		}

		const void PrintBuildLog(cl_int ret, cl_program program, cl_device_id device) {
			size_t returnedBuf = 0;

			// Call it twice to get the size of the buffer
			ret = clGetProgramBuildInfo(
					program,
					device,
					CL_PROGRAM_BUILD_LOG,
					0,
					nullptr,
					&returnedBuf
					);

			std::vector<char> buf(returnedBuf, '\0');

			ret = clGetProgramBuildInfo(
					program,
					device,
					CL_PROGRAM_BUILD_LOG,
					returnedBuf,
					buf.data(),
					&returnedBuf
					);

			clCheckError(ret, "get build log");
			// Returned buf creates a entry for the null terminator
			// so the buffer is never actually 'empty'
			if(buf.size() > 1){
				std::cout << "Log entry for last program build: " << "\n" << buf.data() << "\n";
			}
		}

		const std::string ReadFile(const std::string &fileName) {
			namespace fs = std::filesystem;
			auto path = fs::path(fs::absolute(fileName));

			if(!fs::exists(path)){
				throw KernelNotFound(path.string());
			}

			auto fileStream = std::ifstream(fileName, std::ios::in);

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

			return programSource;
		}
		template<typename T> static void PrintPlatforms(T& out){
					auto platforms = Stateless::GetPlatforms();

					size_t outBufSize = 0;
					int index = 0;
					for(auto platformId : platforms) {
						auto ret = clGetPlatformInfo(platformId, CL_PLATFORM_NAME, 0, nullptr, &outBufSize);
						clCheckError(ret, "Get Platform Info Size", false);

						std::vector<char> outBuf(outBufSize);
						ret = clGetPlatformInfo(platformId, CL_PLATFORM_NAME, outBufSize, outBuf.data(), nullptr);
						clCheckError(ret, "Get Platform Info", false);
						out << index << ".Platform name: " << outBuf.data() << "\n";
						index++;
					}
				}
	} // namespace Statelss

} //End namespace OCLT
