#if !defined(OCL_KERNEL_H)
#define OCL_KERNEL_H

#include <variant>
#include <CL/cl.h>
#include "custom/clext.h"

namespace OCLT{
    class Kernel;
    const std::vector<cl_platform_id> getPlatforms();

    class Input {
        public:
            Input(std::string nam, Kernel* kernel): name(nam), kernel(kernel){}
	        Kernel* Done() {
		        return kernel;
	        }

	    private:
            std::string name;
            Kernel* kernel;
    };
    template<typename T> class InputType : public Input {
        public:
            InputType(std::string name, Kernel* kernel) : Input(name, kernel){}
            InputType<T>* SetBuffer(T* in){
                buffer = in;
                return this;
            }
            InputType<T>* SetBuffer(std::vector<T>&& in){
                buffer = in;
                return this;
            }

        private:
            std::variant<T*, std::vector<T>> buffer;

    };

    class Output {
        public:
            Output(std::string name, Kernel* kernel): name(name), kernel(kernel){}
            Kernel* Done() {
		        return kernel;
	        }
        private:
            std::string name;
            Kernel* kernel;
    };

    template<typename T> class OutputType : public Output {
        public:
            OutputType(std::string name, Kernel* kernel) : Output(name, kernel){}
            OutputType<T>* SetBuffer(T* in){
                buffer = in;
                return this;
            }
            OutputType<T>* SetBuffer(std::vector<T>&& in){
                buffer = in;
                return this;
            }

        private:
            std::variant<T*, std::vector<T>> buffer;
    };

    class CLContext {
        public:
            CLContext(){}
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

            void PrintPlatforms(std::ostream& out){
               auto platforms = getPlatforms(); 

		        size_t outBufSize = 0;
		        for(auto index : platforms){
                    auto ret = clGetPlatformInfo(index, CL_PLATFORM_NAME, 0, nullptr, &outBufSize);
			        clCheckError(ret, "Get Platform Info Size", false);

			        char outBuf[outBufSize];
                    ret = clGetPlatformInfo(index, CL_PLATFORM_NAME, outBufSize, outBuf, nullptr);
			        clCheckError(ret, "Get Platform Info", false);
                    out << "Platform name: " << outBuf << "\n";

                }

            }
            void Find(){

                auto platforms = getPlatforms();
                assert(!platforms.empty());

                auto devices = getDevicesForPlatform(platforms[platformIndex]);
                assert(!devices.empty());

                this->device = devices[deviceIndex];
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
            cl_context context;
            cl_device_id device;
            cl_platform_id platform;
            cl_device_type type;
            int deviceIndex;
            int platformIndex;

    };

    class Kernel {
        public:
            Kernel(std::string name): name(name){}
            Kernel* Source(std::string source){
                this->source = source;
                return this;
            };

            template<typename T> std::shared_ptr<InputType<T>> AddInput(std::string name){
                auto newInput = std::make_shared<InputType<T>>(name, this);
                inputs.push_back(newInput);
                return newInput;
            };
            template<typename T> std::shared_ptr<OutputType<T>> AddOutput(std::string name){
                auto newInput = std::make_shared<OutputType<T>>(name, this);
                outputs.push_back(newInput);
                return newInput;
            };

            Kernel* Build(CLContext& context){
                return this;

            }

        private:
            std::string name;
            std::string source;
            std::vector<std::shared_ptr<Input>> inputs;
            std::vector<std::shared_ptr<Output>> outputs;
    };
    
    const std::vector<cl_platform_id> getPlatforms(){
	    cl_uint numPlatforms = 0;
	    cl_int ret = clGetPlatformIDs(0, nullptr, &numPlatforms);
	    clCheckError(ret, "Get PlatformID", false);

	    std::vector<cl_platform_id> platforms(numPlatforms);
	    ret = clGetPlatformIDs(numPlatforms, platforms.data(), NULL);

        return platforms;
    }

}

#endif // OCL_KERNEL_H
