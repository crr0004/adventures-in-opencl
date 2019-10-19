set( SAMPLE_NAME part_2 )
set( SOURCE_FILES 
		main.cpp 
)


file(GLOB INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h" )

add_executable( ${SAMPLE_NAME} ${SOURCE_FILES} ${INCLUDE_FILES} ${EXTRA_FILES})

target_link_libraries(${SAMPLE_NAME} ${LIBS})

set_target_properties( ${SAMPLE_NAME} PROPERTIES
                        COMPILE_FLAGS ${COMPILER_FLAGS}
                        LINK_FLAGS ${LINKER_FLAGS}
                     )
# Set output directory to bin
target_link_libraries( ${SAMPLE_NAME} ${LIBS})
set(EXECUTABLE_OUTPUT_PATH bin)
