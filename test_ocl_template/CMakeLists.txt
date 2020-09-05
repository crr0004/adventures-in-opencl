set( SAMPLE_NAME test_ocl_template )
set( SOURCE_FILES 
		src/test.cpp
		src/kernel.cpp 
)


file(GLOB INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h" )
#set(CMAKE_CXX_CLANG_TIDY clang-tidy -header-filter=${CMAKE_SOURCE_DIR}/shared/ocl_template/*
#	-checks=readability-*,cppcoreguidelines-*,bugprone-*,clang-*,llvm-*,performance-*,readability-*,modernize-*)

add_executable( ${SAMPLE_NAME} ${SOURCE_FILES} ${INCLUDE_FILES} ${EXTRA_FILES})

target_link_libraries(${SAMPLE_NAME} ${LIBS})

# Disable unused variables for tests because we often use the tests
# to test for compile errors
set_target_properties( ${SAMPLE_NAME} PROPERTIES
	COMPILE_FLAGS "${COMPILER_FLAGS} -g -Wall -Werror -Wno-unused-variable"
                        LINK_FLAGS ${LINKER_FLAGS}
                     )
# Set output directory to bin
target_link_libraries( ${SAMPLE_NAME} ${LIBS})
set(EXECUTABLE_OUTPUT_PATH bin)
