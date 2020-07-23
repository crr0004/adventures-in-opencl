cmake_minimum_required( VERSION 2.6.0 )
project(adventures_in_opencl)
message(STATUS "MSVC set: " ${MSVC})

# Disable testing in the benchmarking library
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
SET(HAVE_CXX_FLAG_STD_CXX11 ON CACHE BOOL "")
add_subdirectory(benchmark)
# set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE BOOL "" FORCE)

# Set CMAKE_BUILD_TYPE (default = Release)
# if("${CMAKE_BUILD_TYPE}" STREQUAL "")
# endif()



############################################################################
# Find OpenCL include and libs
message(STATUS ${CMAKE_SOURCE_DIR})
find_path( OPENCL_INCLUDE_DIRS 
NAMES OpenCL/cl.h CL/cl.h
HINTS
	ENV OPENCL_PATH
	${CMAKE_SOURCE_DIR}/shared/system/
PATH_SUFFIXES
	include
)
mark_as_advanced(OPENCL_INCLUDE_DIRS)

find_library(OPENCL_LIBRARIES
	NAMES OpenCL libOpenCL.so libOpenCL.so.1 OpenCL.dll
	PATH_SUFFIXES
	lib/x86_64/
	HINTS ENV OPENCL_PATH
)

############################################################################
message(STATUS "Using " ${OPENCL_LIBRARIES})
message(STATUS "Using " ${OPENCL_INCLUDE_DIRS})

# no-omit-frame-pointer for perf reporting and frame tracing
set( COMPILER_FLAGS "-std=c++20 -fpermissive -fno-omit-frame-pointer" )
set( LINKER_FLAGS " " )
set( ADDITIONAL_LIBRARIES "" )


set(LIBS ${OPENCL_LIBRARIES} ${ADDITIONAL_LIBRARIES} benchmark)
message(STATUS "Including " ${OPENCL_INCLUDE_DIRS} " on all targets")
message(STATUS "Using OpenCL library " ${OPENCL_LIBRARIES})
include_directories(${CMAKE_SOURCE_DIR}/shared/custom)
include_directories(${CMAKE_SOURCE_DIR}/shared/)
include_directories(SYSTEM ${OPENCL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/benchmark/include)

add_subdirectory(naive_approach)
add_subdirectory(summing_numbers)
add_subdirectory(part_2)
add_subdirectory(reduce_optimisation)
add_subdirectory(test_ocl_template)
