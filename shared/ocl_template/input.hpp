#ifndef OCL_INPUT_H
#define OCL_INPUT_H

#include <CL/cl.h>
#include <memory>
#include <string>

namespace OCLT{
/*
class KernelBuffer {
	public:
		typedef std::unique_ptr<KernelBuffer> Ptr;
		typedef std::shared_ptr<KernelBuffer> SPtr;
		KernelBuffer(std::string nam, Kernel *kernel) : name(nam), kernel(kernel) { }
		friend class Kernel;
		Kernel* Done() {
			return kernel;
		}
		virtual ~KernelBuffer() { }


	protected:
		virtual BuiltBuffer Create(BuiltCLContext, Kernel *kernel) = 0;

	private:
		std::string name;
		Kernel* kernel;
};

template<typename T> struct InputBuffer{
	std::shared_ptr<T*> in;
	size_t size;

};
class BuiltBuffer {
	public:
		BuiltBuffer(cl_mem mem) : memObject(mem){}
		~BuiltBuffer(){
			clReleaseMemObject(memObject);
		};
		friend class Kernel;
	private:
		cl_mem memObject;
};

template <typename T> class TypedKernelBuffer : public KernelBuffer {
	public:
		TypedKernelBuffer(std::string name, Kernel *kernel) : Input(name, kernel) {
		}
		// how do we enforce this pointer to live until we use it?
		// some smart pointer?
		TypedKernelBuffer<T> *SetBuffer(InputBuffer<T> buffer) {
			buffer = buffer;
			return this;
		}
		TypedKernelBuffer<T> *SetKernelIndex(unsigned int index) {
			this->index = index;
		}
		TypedKernelBuffer<T> *SetMemFlags(cl_mem_flags flags) {
			this->flags = flags;
		}
		~TypedKernelBuffer(){

		}
		friend class Kernel;
	protected:
		BuiltBuffer Create(BuiltCLContext *context, Kernel *kernel) {
			cl_int ret;
			//				void* memPointer;

			cl_mem memObject = clCreateBuffer(context->getContext(), flags, buffer.size, nullptr, &ret);
			clCheckError(ret, "Create buffer", false);

			return BuiltBuffer{ memObject };
		}
		void Buffer(cl_command_queue queue, BuiltKernel& kernel, BuiltBuffer& builtBuffer){
			cl_int ret = clSetKernelArg(kernel.kernel, index, sizeof(cl_mem), (void *)&builtBuffer.memObject);
			clCheckError(ret, "Set local buffer");

			clEnqueueFillBuffer(queue, builtBuffer.memObject, CL_FALSE, 0, buffer.size, buffer.in.get(), 0, nullptr, nullptr);



		}
	private:
		cl_mem_flags flags;
		unsigned int index;
		InputBuffer<T> buffer;
};


*/
} // namespace OCLT
#endif
