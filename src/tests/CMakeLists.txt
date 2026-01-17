# Tests CMakeLists.txt for simple-tftpd
# Copyright 2024 SimpleDaemons
# Licensed under Apache License 2.0

# Enable testing
enable_testing()

# Find Google Test if available
find_package(GTest QUIET)
if(NOT GTest_FOUND)
    # Try to find GTest without find_package
    find_library(GTEST_LIBRARIES gtest gtest_main)
    find_path(GTEST_INCLUDE_DIRS gtest/gtest.h)
    if(GTEST_LIBRARIES AND GTEST_INCLUDE_DIRS)
        set(GTest_FOUND TRUE)
        set(GTEST_LIBRARIES ${GTEST_LIBRARIES})
        set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS})
    endif()
endif()

if(GTest_FOUND)
    message(STATUS "Found Google Test: ${GTEST_LIBRARIES}")
    
    # Unit test source files
    set(UNIT_TEST_SOURCES
        test_main.cpp
        unit/packet_tests.cpp
        unit/connection_tests.cpp
        unit/config_tests.cpp
        unit/security_tests.cpp
        unit/monitoring_tests.cpp
        utils/test_helpers.cpp
    )
    
    # Unit test executable
    add_executable(simple-tftpd-tests ${UNIT_TEST_SOURCES})
    
    # Link against the core library
    target_link_libraries(simple-tftpd-tests 
        simple-tftpd-core
        ${GTEST_LIBRARIES}
        ${PLATFORM_LIBRARIES}
    )
    
    if(JSONCPP_FOUND)
        target_link_libraries(simple-tftpd-tests ${JSONCPP_LIBRARIES})
    endif()
    
    # Include directories
    target_include_directories(simple-tftpd-tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GTEST_INCLUDE_DIRS}
    )
    
    # Add unit tests
    add_test(NAME simple-tftpd-unit-tests COMMAND simple-tftpd-tests)
    
    # Set test properties
    set_tests_properties(simple-tftpd-unit-tests PROPERTIES
        TIMEOUT 300
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    # Valgrind support for memory leak detection
    if(ENABLE_VALGRIND)
        find_program(VALGRIND_EXECUTABLE valgrind)
        if(VALGRIND_EXECUTABLE)
            add_test(NAME simple-tftpd-unit-tests-valgrind
                COMMAND ${VALGRIND_EXECUTABLE} 
                    --leak-check=full 
                    --show-leak-kinds=all 
                    --track-origins=yes 
                    --error-exitcode=1
                    $<TARGET_FILE:simple-tftpd-tests>)
            set_tests_properties(simple-tftpd-unit-tests-valgrind PROPERTIES
                TIMEOUT 600
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            )
            message(STATUS "Valgrind memory leak detection enabled for tests")
        else()
            message(WARNING "Valgrind not found. Install with: sudo apt-get install valgrind")
        endif()
    endif()
    
    # Integration test source files
    set(INTEGRATION_TEST_SOURCES
        integration/integration_tests.cpp
        integration/performance_tests.cpp
        integration/tftp_client.cpp
        utils/test_helpers.cpp
    )
    
    # Integration test executable
    add_executable(simple-tftpd-integration-tests ${INTEGRATION_TEST_SOURCES})
    
    # Link against the core library
    target_link_libraries(simple-tftpd-integration-tests 
        simple-tftpd-core
        ${GTEST_LIBRARIES}
        ${PLATFORM_LIBRARIES}
    )
    
    if(JSONCPP_FOUND)
        target_link_libraries(simple-tftpd-integration-tests ${JSONCPP_LIBRARIES})
    endif()
    
    # Include directories for integration tests
    target_include_directories(simple-tftpd-integration-tests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/integration
        ${GTEST_INCLUDE_DIRS}
    )
    
    # Add integration tests
    add_test(NAME simple-tftpd-integration-tests COMMAND simple-tftpd-integration-tests)
    
    # Set test properties
    set_tests_properties(simple-tftpd-integration-tests PROPERTIES
        TIMEOUT 600
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
else()
    message(WARNING "Google Test not found. Tests will not be built.")
endif()
