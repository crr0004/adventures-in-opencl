cmake_minimum_required( VERSION 2.6.0 )
project(naive_approach)

set( SAMPLE_NAME main )
set( SOURCE_FILES 
		main.cpp 
)


# Set CMAKE_BUILD_TYPE (default = Release)
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
set(CMAKE_BUILD_TYPE Debug)
endif()

############################################################################
# Find OpenCL include and libs
message(STATUS ${CMAKE_SOURCE_DIR})
find_path( OPENCL_INCLUDE_DIRS 
NAMES OpenCL/cl.h CL/cl.h
HINTS
	ENV OPENCL_PATH
	${CRR_OPENCL_INCLUDE_PATH}
	${CMAKE_SOURCE_DIR}/../shared
PATH_SUFFIXES
	include
)
mark_as_advanced(OPENCL_INCLUDE_DIRS)

find_library(OPENCL_LIBRARIES
	NAMES OpenCL libOpenCL.so
	PATH_SUFFIXES
	lib/x86_64/
	/opt/amdgpu-pro/lib64/
	HINTS ENV OPENCL_PATH
)

############################################################################
message(STATUS "Using " ${OPENCL_LIBRARIES})
message(STATUS "Using " ${OPENCL_INCLUDE_DIRS})

set( COMPILER_FLAGS "-fpermissive" )
set( LINKER_FLAGS " " )
set( ADDITIONAL_LIBRARIES "" )
set(LIBS ${OPENCL_LIBRARIES} ${ADDITIONAL_LIBRARIES})
message(STATUS "Including " ${OPENCL_INCLUDE_DIRS} " on all targets")
message(STATUS "Using OpenCL library " ${OPENCL_LIBRARIES})
include_directories(SYSTEM ${OPENCL_INCLUDE_DIRS})

file(GLOB INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h" )

add_executable( ${SAMPLE_NAME} ${SOURCE_FILES} ${INCLUDE_FILES} ${EXTRA_FILES})

target_link_libraries(${SAMPLE_NAME} ${LIBS})

set_target_properties( ${SAMPLE_NAME} PROPERTIES
                        COMPILE_FLAGS ${COMPILER_FLAGS}
                        LINK_FLAGS ${LINKER_FLAGS}
                     )
# Set output directory to bin
set(EXECUTABLE_OUTPUT_PATH bin)
target_link_libraries( ${SAMPLE_NAME} ${OPENCL_LIBRARIES} ${ADDITIONAL_LIBRARIES} )

# Set output directory to bin
set(EXECUTABLE_OUTPUT_PATH bin/)
