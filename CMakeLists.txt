cmake_minimum_required(VERSION 3.16)
# Get system architecture for package naming
execute_process(COMMAND uname -m OUTPUT_VARIABLE SYSTEM_ARCH OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "System architecture: ${SYSTEM_ARCH}")

project(simple-tftpd-${SYSTEM_ARCH} VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0" CACHE STRING "Minimum macOS deployment version" FORCE)
    # Allow command line override of architectures
    if(NOT DEFINED CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "Build architectures for macOS" FORCE)
    endif()
    set(PLATFORM_MACOS TRUE)
    message(STATUS "Building for macOS (Monterey 12.0+) with architectures: ${CMAKE_OSX_ARCHITECTURES}")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
    message(STATUS "Building for Linux")
elseif(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    message(STATUS "Building for Windows")
    
    # Windows-specific settings
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    
    # Set Windows SDK version if available
    if(DEFINED ENV{WINDOWS_SDK_VERSION})
        set(CMAKE_SYSTEM_VERSION $ENV{WINDOWS_SDK_VERSION})
    endif()
endif()

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" OFF)
option(ENABLE_LOGGING "Enable logging" ON)
option(ENABLE_IPV6 "Enable IPv6 support" ON)
option(USE_SYSTEM_LIBS "Use system libraries instead of Homebrew" OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find jsoncpp for JSON parsing (optional for now)
# Temporarily disabled to ensure basic build works
# find_package(PkgConfig QUIET)
# if(PkgConfig_FOUND)
#     pkg_check_modules(JSONCPP jsoncpp)
# endif()
# 
# if(NOT JSONCPP_FOUND)
#     # Try to find jsoncpp without pkg-config
#     find_library(JSONCPP_LIBRARIES jsoncpp)
#     find_path(JSONCPP_INCLUDE_DIRS json/json.h)
#     if(JSONCPP_LIBRARIES AND JSONCPP_INCLUDE_DIRS)
#         set(JSONCPP_FOUND TRUE)
#         set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARIES})
#         set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIRS})
# endif()
# endif()

set(JSONCPP_FOUND FALSE)
message(STATUS "jsoncpp temporarily disabled for basic build")
add_definitions(-DDISABLE_JSON_CONFIG)

# Platform-specific library handling
if(PLATFORM_MACOS)
    # macOS specific libraries
    find_library(SECURITY_FRAMEWORK Security)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(SYSTEM_CONFIGURATION_FRAMEWORK SystemConfiguration)
    
    # macOS doesn't need separate socket or rt libraries
    set(PLATFORM_LIBRARIES 
        ${SECURITY_FRAMEWORK}
        ${CORE_FOUNDATION_FRAMEWORK}
        ${SYSTEM_CONFIGURATION_FRAMEWORK}
    )
elseif(PLATFORM_LINUX)
    # Linux specific libraries
    find_library(RT_LIBRARY rt)
    set(PLATFORM_LIBRARIES ${RT_LIBRARY})
elseif(PLATFORM_WINDOWS)
    # Windows specific libraries
    set(PLATFORM_LIBRARIES ws2_32 iphlpapi)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
if(JSONCPP_FOUND)
    include_directories(${JSONCPP_INCLUDE_DIRS})
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/core/tftp_server.cpp
    src/core/tftp_connection.cpp
    src/core/tftp_packet.cpp
    src/utils/logger.cpp
    src/utils/config_parser.cpp
)

# Core library source files (without main.cpp)
set(CORE_SOURCES
    src/core/tftp_server.cpp
    src/core/tftp_connection.cpp
    src/core/tftp_packet.cpp
    src/utils/logger.cpp
    src/utils/config_parser.cpp
)

# Header files
set(HEADERS
    include/simple_tftpd/tftp_server.hpp
    include/simple_tftpd/tftp_connection.hpp
    include/simple_tftpd/tftp_packet.hpp
    include/simple_tftpd/tftp_config.hpp
    include/simple_tftpd/logger.hpp
    include/simple_tftpd/platform.hpp
)

# Create core library
add_library(simple-tftpd-core STATIC ${CORE_SOURCES} ${HEADERS})

# Link libraries for core library
target_link_libraries(simple-tftpd-core ${PLATFORM_LIBRARIES})
if(JSONCPP_FOUND)
    target_link_libraries(simple-tftpd-core ${JSONCPP_LIBRARIES})
endif()

# Set properties for core library
set_target_properties(simple-tftpd-core PROPERTIES
    OUTPUT_NAME "simple-tftpd-core"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Create executable
add_executable(simple-tftpd src/main.cpp)

# Link executable against core library
target_link_libraries(simple-tftpd simple-tftpd-core)

# Set properties
set_target_properties(simple-tftpd PROPERTIES
    OUTPUT_NAME "simple-tftpd"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Install rules
install(TARGETS simple-tftpd
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install configuration files
install(DIRECTORY config/ DESTINATION etc/simple-tftpd
    FILES_MATCHING PATTERN "*.conf" PATTERN "*.example"
)

# Install documentation
install(DIRECTORY docs/ DESTINATION share/doc/simple-tftpd)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(src/tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(src/examples)
endif()

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/simple-tftpd-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/simple-tftpd-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/simple-tftpd-config.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/simple-tftpd-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/simple-tftpd-config-version.cmake"
    DESTINATION lib/cmake/simple-tftpd
)

# CPack configuration for macOS packaging
if(PLATFORM_MACOS)
    set(CPACK_GENERATOR "DragNDrop;productbuild")
    set(CPACK_PACKAGE_NAME "simple-tftpd")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Simple TFTP Daemon for macOS")
    set(CPACK_PACKAGE_VENDOR "simple-tftpd")
    set(CPACK_PACKAGE_CONTACT "SimpleDaemons")
    set(CPACK_PACKAGE_FILE_NAME "simple-tftpd-${PROJECT_VERSION}-macOS-${SYSTEM_ARCH}")
    set(CPACK_DMG_VOLUME_NAME "simple-tftpd-${PROJECT_VERSION}-macOS-${SYSTEM_ARCH}")
    set(CPACK_DMG_FORMAT "UDZO")
    
    # PKG configuration
    set(CPACK_PRODUCTBUILD_PACKAGE_NAME "simple-tftpd-${PROJECT_VERSION}-macOS-${SYSTEM_ARCH}")
    set(CPACK_PRODUCTBUILD_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PRODUCTBUILD_PACKAGE_DESCRIPTION "Simple TFTP Daemon for macOS - ${SYSTEM_ARCH}")
    set(CPACK_PRODUCTBUILD_PACKAGE_VENDOR "simple-tftpd")
    set(CPACK_PRODUCTBUILD_PACKAGE_CONTACT "SimpleDaemons")
    
    include(CPack)
endif()
