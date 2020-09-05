#ifndef OCL_EXCEPTIONS_H
#define OCL_EXCEPTIONS_H

#include <stdexcept>
#include <string>
namespace OCLT{
class Exception : std::exception {
	std::string message;
	Exception() = delete;
	public:
		Exception(std::string message) : std::exception() {
			message = "Something happened " + message;
		}
	const char *what() const noexcept {
		return message.c_str();
	}
};
class NoPlatformsFound : Exception {
	public:
	NoPlatformsFound()
	: Exception("No platforms found for OpenCL. Try plugging in a device and checking it is listed "
	            "in clinfo") {
	}
};
class NoDevicesFound : Exception {
	public:
	NoDevicesFound()
	: Exception(
	  "No devices found for chosen platform. Try checking devices are being detected in clinfo") {
	}
};
class KernelNotFound : Exception {
	public:
	KernelNotFound(std::string fileName) : Exception("hello") {
		helpMessage = "File could not be found at " + fileName;
	}
	const char *what() const noexcept {
		return helpMessage.c_str();
	}

	private:
	std::string helpMessage;
};
class BufferSizeMustBeGreaterThanZero : Exception {
	public:
		BufferSizeMustBeGreaterThanZero() : Exception("Make sure to set the size when building a buffer"){}

};
class PlatformIndexOutOfRange : Exception {
	public:
		// Really need to find a better way to format these messages
		PlatformIndexOutOfRange(int platformIndex, size_t platformCount) : Exception("Could not find the platform in the provided index " + std::to_string(platformIndex) + "/" + std::to_string(platformCount)){}

};
}

#endif
