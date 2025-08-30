# Examples CMakeLists.txt for simple-tftpd
# Copyright 2024 SimpleDaemons
# Licensed under Apache License 2.0

# Example source files
set(EXAMPLE_SOURCES
    basic_server_example.cpp
    config_example.cpp
)

# Create example executables
foreach(example_source ${EXAMPLE_SOURCES})
    get_filename_component(example_name ${example_source} NAME_WE)
    
    add_executable(${example_name} ${example_source})
    
    # Link libraries
    target_link_libraries(${example_name} ${PLATFORM_LIBRARIES})
    
    if(JSONCPP_FOUND)
        target_link_libraries(${example_name} ${JSONCPP_LIBRARIES})
    endif()
    
    # Include directories
    target_include_directories(${example_name} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )
    
    # Set output directory
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endforeach()
