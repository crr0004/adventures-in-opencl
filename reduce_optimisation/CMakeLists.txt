set( SAMPLE_NAME reduce_optimisation )
set( SOURCE_FILES 
	benchmark_runner.cpp
)
add_executable( ${SAMPLE_NAME} ${SOURCE_FILES} ${INCLUDE_FILES} ${EXTRA_FILES})
target_include_directories(${SAMPLE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${SAMPLE_NAME} ${LIBS})

set_target_properties( ${SAMPLE_NAME} PROPERTIES
												COMPILE_FLAGS ${COMPILER_FLAGS} 
                        LINK_FLAGS ${LINKER_FLAGS}
                     )
# Set output directory to bin
target_link_libraries( ${SAMPLE_NAME} ${LIBS})
set(EXECUTABLE_OUTPUT_PATH bin)
