cmake_minimum_required( VERSION 2.6.0 )
project(adventures_in_opencl)

# Disable testing in the benchmarking library
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
add_subdirectory(benchmark)

# Set CMAKE_BUILD_TYPE (default = Release)
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE BOOL "" FORCE)
endif()


############################################################################
# Find OpenCL include and libs
message(STATUS ${CMAKE_SOURCE_DIR})
find_path( OPENCL_INCLUDE_DIRS 
NAMES OpenCL/cl.h CL/cl.h
HINTS
	ENV OPENCL_PATH
	${CRR_OPENCL_INCLUDE_PATH}
	${CMAKE_SOURCE_DIR}/shared/system/
PATH_SUFFIXES
	include
)
mark_as_advanced(OPENCL_INCLUDE_DIRS)

find_library(OPENCL_LIBRARIES
	NAMES OpenCL libOpenCL.so libOpenCL.so.1
	PATH_SUFFIXES
	lib/x86_64/
	/opt/amdgpu-pro/lib64/
	HINTS ENV OPENCL_PATH
)

############################################################################
message(STATUS "Using " ${OPENCL_LIBRARIES})
message(STATUS "Using " ${OPENCL_INCLUDE_DIRS})

set( COMPILER_FLAGS "-fpermissive " )
set( LINKER_FLAGS " " )
set( ADDITIONAL_LIBRARIES "" )
set(LIBS ${OPENCL_LIBRARIES} ${ADDITIONAL_LIBRARIES} benchmark)
message(STATUS "Including " ${OPENCL_INCLUDE_DIRS} " on all targets")
message(STATUS "Using OpenCL library " ${OPENCL_LIBRARIES})
include_directories(${CMAKE_SOURCE_DIR}/shared/custom)
include_directories(SYSTEM ${OPENCL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/benchmark/include)

add_subdirectory(naive_approach)
add_subdirectory(summing_numbers)
add_subdirectory(part_2)
