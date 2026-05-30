# CMakeLists.txt template for SimpleDaemons projects
# Simple TFTP Daemon - A lightweight and secure TFTP server
# Copyright 2024 SimpleDaemons

cmake_minimum_required(VERSION 3.16)
project(simple-tftpd VERSION 0.3.0 LANGUAGES CXX DESCRIPTION "A lightweight, cross-platform TFTP daemon")

# FHS install paths (/usr/bin, /etc/simple-tftpd, …)
include(GNUInstallDirs)
if(UNIX AND NOT APPLE)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Installation prefix" FORCE)
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(PLATFORM_NAME "FreeBSD")
    add_compile_definitions(SIMPLE_TFTPD_FREEBSD=1)
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
endif()

# Version selection
set(BUILD_VERSION "production" CACHE STRING "Version to build: production, enterprise, datacenter")
set_property(CACHE BUILD_VERSION PROPERTY STRINGS production enterprise datacenter)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_TESTS "Enable tests" ON)
option(ENABLE_PACKAGING "Enable package generation" ON)
option(ENABLE_SSL "Enable SSL/TLS support" ON)
option(ENABLE_JSON "Enable JSON support" ON)
option(ENABLE_STATIC_LINKING "Enable static linking for self-contained binaries" OFF)
option(ENABLE_SANITIZER "Enable AddressSanitizer for memory leak detection" OFF)
option(ENABLE_VALGRIND "Enable Valgrind support in tests" OFF)

# Find required packages
find_package(Threads REQUIRED)

if(ENABLE_SSL)
    find_package(OpenSSL REQUIRED)
endif()

if(ENABLE_JSON)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JSONCPP REQUIRED jsoncpp)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core sources (always included - shared by all versions)
set(CORE_SOURCES
    src/core/tftp/server.cpp
    src/core/tftp/connection.cpp
    src/core/tftp/packet.cpp
    src/core/tftp/monitoring.cpp
    src/core/config/parser.cpp
    src/core/utils/logger.cpp
)

# Core headers
file(GLOB_RECURSE CORE_HEADERS "include/simple-tftpd/core/*.hpp")

# Version-specific sources and main
if(BUILD_VERSION STREQUAL "production")
    set(VERSION_SOURCES
        ${CORE_SOURCES}
        src/production/security/manager.cpp
        # Production-specific sources
        # src/production/features/advanced_manager.cpp
    )
    file(GLOB_RECURSE VERSION_HEADERS
        "include/simple-tftpd/core/*.hpp"
        "include/simple-tftpd/production/*.hpp"
    )
    set(VERSION_MAIN main/production.cpp)
    set(PACKAGE_SUFFIX "production")
    set(PROJECT_DESCRIPTION "Simple TFTP Daemon - Production Version")

elseif(BUILD_VERSION STREQUAL "enterprise")
    set(VERSION_SOURCES
        ${CORE_SOURCES}
        # Production sources
        # Enterprise sources will be added here
        # src/enterprise/ha/failover.cpp
        # src/enterprise/management/web_ui.cpp
    )
    file(GLOB_RECURSE VERSION_HEADERS
        "include/simple-tftpd/core/*.hpp"
        "include/simple-tftpd/production/*.hpp"
        "include/simple-tftpd/enterprise/*.hpp"
    )
    set(VERSION_MAIN main/enterprise.cpp)
    set(PACKAGE_SUFFIX "enterprise")
    set(PROJECT_DESCRIPTION "Simple TFTP Daemon - Enterprise Version")

elseif(BUILD_VERSION STREQUAL "datacenter")
    set(VERSION_SOURCES
        ${CORE_SOURCES}
        # Production sources
        # Enterprise sources (when implemented)
        # Datacenter sources will be added here
        # src/datacenter/cluster/cluster_manager.cpp
        # src/datacenter/multi-site/sync.cpp
    )
    file(GLOB_RECURSE VERSION_HEADERS
        "include/simple-tftpd/core/*.hpp"
        "include/simple-tftpd/production/*.hpp"
        "include/simple-tftpd/enterprise/*.hpp"
        "include/simple-tftpd/datacenter/*.hpp"
    )
    set(VERSION_MAIN main/datacenter.cpp)
    set(PACKAGE_SUFFIX "datacenter")
    set(PROJECT_DESCRIPTION "Simple TFTP Daemon - Datacenter Version")

else()
    message(FATAL_ERROR "Unknown BUILD_VERSION: ${BUILD_VERSION}. Must be: production, enterprise, or datacenter")
endif()

# AddressSanitizer support (for memory leak detection)
if(ENABLE_SANITIZER)
    set(ASAN_FLAGS "-fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
    message(STATUS "AddressSanitizer enabled for memory leak detection")
endif()

# Create library
add_library(${PROJECT_NAME}_lib STATIC ${VERSION_SOURCES} ${VERSION_HEADERS})

# Alias for tests (they link to simple-tftpd-core)
add_library(simple-tftpd-core ALIAS ${PROJECT_NAME}_lib)

# Create executable
add_executable(${PROJECT_NAME} ${VERSION_MAIN})

# Link libraries (use keyword signature consistently for target_link_libraries)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_lib)
target_link_libraries(${PROJECT_NAME}_lib PUBLIC Threads::Threads)

if(ENABLE_SSL)
    target_link_libraries(${PROJECT_NAME}_lib PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()

if(ENABLE_JSON)
    set(JSONCPP_LINK_LIB "")
    if(ENABLE_STATIC_LINKING)
        find_library(JSONCPP_LINK_LIB NAMES libjsoncpp.a jsoncpp
            HINTS ${JSONCPP_LIBRARY_DIRS}
            PATHS /opt/local/lib /usr/local/lib /usr/lib64 /usr/lib)
    endif()
    if(NOT JSONCPP_LINK_LIB)
        set(JSONCPP_LINK_LIB ${JSONCPP_LIBRARIES})
    endif()

    target_link_libraries(${PROJECT_NAME}_lib PUBLIC ${JSONCPP_LINK_LIB})
    target_link_directories(${PROJECT_NAME}_lib PUBLIC ${JSONCPP_LIBRARY_DIRS})
    target_include_directories(${PROJECT_NAME}_lib PUBLIC ${JSONCPP_INCLUDE_DIRS})
    target_compile_options(${PROJECT_NAME}_lib PRIVATE ${JSONCPP_CFLAGS_OTHER})
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_include_directories(${PROJECT_NAME}_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Detect static libstdc++ for GCC (on EL9 the .a is in CRB: /usr/lib/gcc/.../libstdc++.a)
set(HAVE_STATIC_LIBSTDCXX FALSE)
if(ENABLE_STATIC_LINKING AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++.a
        OUTPUT_VARIABLE _stdcpp_static_candidate
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(IS_ABSOLUTE "${_stdcpp_static_candidate}" AND EXISTS "${_stdcpp_static_candidate}")
        set(STDCPP_STATIC_LIB "${_stdcpp_static_candidate}")
    else()
        find_library(STDCPP_STATIC_LIB NAMES libstdc++.a
            PATHS /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib)
        if(NOT STDCPP_STATIC_LIB OR NOT STDCPP_STATIC_LIB MATCHES "\\.a$")
            file(GLOB _gcc_lib_dirs LIST_DIRECTORIES true "/usr/lib/gcc/*/*")
            find_library(STDCPP_STATIC_LIB NAMES libstdc++.a PATHS ${_gcc_lib_dirs})
        endif()
    endif()
    if(STDCPP_STATIC_LIB AND EXISTS "${STDCPP_STATIC_LIB}")
        set(HAVE_STATIC_LIBSTDCXX TRUE)
        message(STATUS "Static libstdc++: ${STDCPP_STATIC_LIB}")
    else()
        message(WARNING
            "Static libstdc++ not found; linking libstdc++ dynamically.\n"
            "  EL9/Rocky/Alma: sudo crb enable && sudo dnf install libstdc++-static\n"
            "  EL8:            sudo dnf config-manager --set-enabled powertools && sudo dnf install libstdc++-static\n"
            "  Debian/Ubuntu:  sudo apt-get install libstdc++-static\n"
            "  Or run: make deps")
    endif()
endif()

# Compiler-specific options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX)
    if(ENABLE_STATIC_LINKING)
        target_compile_options(${PROJECT_NAME} PRIVATE /MT)
    endif()
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-private-field -Wno-reorder-ctor -Wno-unused-but-set-variable)
    if(ENABLE_STATIC_LINKING AND HAVE_STATIC_LIBSTDCXX)
        target_compile_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
        target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif()
endif()

# Static linking configuration
if(ENABLE_STATIC_LINKING)
    set(BUILD_SHARED_LIBS OFF)
    if(ENABLE_SSL)
        # Force static linking for OpenSSL
        set(OPENSSL_USE_STATIC_LIBS TRUE)
    endif()
    if(ENABLE_JSON)
        # Force static linking for jsoncpp
        set(JSONCPP_USE_STATIC_LIBS TRUE)
    endif()
endif()

# Install targets
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(FILES LICENSE README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# Linux: explicit FHS paths (/etc, /usr/lib/systemd) so CPack DEB includes them
if(UNIX AND NOT APPLE)
    install(FILES config/simple-tftpd.conf.example
        DESTINATION /etc/${PROJECT_NAME}
    )
    install(FILES deployment/configs/simple-tftpd.conf
        DESTINATION /etc/${PROJECT_NAME}
        RENAME simple-tftpd.conf.example.json
    )
    install(DIRECTORY config/examples/
        DESTINATION /etc/${PROJECT_NAME}/examples
        FILES_MATCHING PATTERN "*.example"
    )
    install(FILES
        deployment/systemd/${PROJECT_NAME}.service
        deployment/systemd/${PROJECT_NAME}@.service
        DESTINATION /usr/lib/systemd/system
    )
    install(PROGRAMS deployment/init.d/${PROJECT_NAME}
        DESTINATION /etc/init.d
    )
elseif(APPLE)
    install(FILES config/simple-tftpd.conf.example
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}
    )
    install(DIRECTORY config/examples/
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/examples
        FILES_MATCHING PATTERN "*.example"
    )
    # Install launchd plist file
    # Use relative path for package creation - CPack will handle proper installation path
    install(FILES deployment/launchd/com.simpledaemons.simple-tftpd.plist
        DESTINATION Library/LaunchDaemons
    )

    # Install docs directory for macOS DMG/PKG packages
    install(DIRECTORY docs/
        DESTINATION docs
        FILES_MATCHING
        PATTERN "*.md"
        PATTERN "*.txt"
        PATTERN "*.html"
    )
endif()

# Tests (expose variables for test subdirectory)
if(ENABLE_TESTS)
    if(ENABLE_JSON)
        set(JSONCPP_FOUND TRUE)
    endif()
    set(PLATFORM_LIBRARIES Threads::Threads)
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/tests/CMakeLists.txt")
        add_subdirectory(src/tests)
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
        add_subdirectory(tests)
    else()
        message(STATUS "No tests directory found, skipping tests")
    endif()
endif()

# Package generation
if(ENABLE_PACKAGING)

    # Package information
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VENDOR "SimpleDaemons")
    if(PROJECT_DESCRIPTION)
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
    else()
        set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A lightweight, cross-platform TFTP daemon")
    endif()
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_CONTACT "info@simpledaemons.com")
    set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/simpledaemons/${PROJECT_NAME}")

    # Package files
    # Use .txt files for macOS PKG (required format), .md for other platforms
    # All packaging resources are organized in the packaging/ directory
    if(APPLE)
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/LICENSE.txt")
        set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/packaging/README.txt")
        set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/packaging/WELCOME.txt")
    else()
        set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
        set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    endif()

    # Platform-specific package settings
    if(WIN32)
        # Windows MSI and ZIP packages
        set(CPACK_GENERATOR "WIX;ZIP")

        # WIX (MSI) settings
        set(CPACK_WIX_PRODUCT_GUID "12345678-1234-1234-1234-123456789012")
        set(CPACK_WIX_UPGRADE_GUID "87654321-4321-4321-4321-210987654321")
        set(CPACK_WIX_PROPERTY_ARPPRODUCTICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico")
        set(CPACK_WIX_PROPERTY_ARPHELPLINK "https://github.com/simpledaemons/${PROJECT_NAME}")
        set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "https://github.com/simpledaemons/${PROJECT_NAME}")
        set(CPACK_WIX_PROPERTY_ARPNOREPAIR "1")
        set(CPACK_WIX_PROPERTY_ARPNOMODIFY "1")

        # ZIP package settings
        set(CPACK_ZIP_COMPONENT_INSTALL ON)

    elseif(APPLE)
        # macOS DMG and PKG packages
        set(CPACK_GENERATOR "DragNDrop;productbuild")

        # DMG settings
        set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
        set(CPACK_DMG_FORMAT "UDBZ")

        # Optional DMG customization files (only set if they exist)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/dmg_background.png")
            set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/assets/dmg_background.png")
        endif()

        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/DS_Store")
            set(CPACK_DMG_DS_STORE "${CMAKE_CURRENT_SOURCE_DIR}/assets/DS_Store")
        endif()

        # PKG settings
        set(CPACK_PRODUCTBUILD_IDENTIFIER "com.simpledaemons.${PROJECT_NAME}")
        set(CPACK_PRODUCTBUILD_PACKAGE_NAME "${PROJECT_NAME}")

    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        set(CPACK_GENERATOR "FREEBSD")
        set(CPACK_PACKAGING_INSTALL_PREFIX "/")
        set(CPACK_SET_DESTDIR TRUE)
        set(CPACK_FREEBSD_PACKAGE_MAINTAINER "SimpleDaemons <info@simpledaemons.com>")
        set(CPACK_FREEBSD_PACKAGE_ORIGIN "net/${PROJECT_NAME}")
        set(CPACK_FREEBSD_PACKAGE_LICENSE "APACHE20")
        if(ENABLE_STATIC_LINKING)
            set(CPACK_FREEBSD_PACKAGE_RUNTIME_DEPENDS "")
        else()
            set(CPACK_FREEBSD_PACKAGE_RUNTIME_DEPENDS "openssl jsoncpp")
        endif()

    else()
        # Linux DEB and RPM packages
        set(CPACK_GENERATOR "DEB;RPM")

        # DEB package settings
        set(CPACK_DEBIAN_PACKAGE_MAINTAINER "SimpleDaemons <info@simpledaemons.com>")
        set(CPACK_DEBIAN_PACKAGE_SECTION "net")
        set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
        # Detect runtime deps from the installed binary (libssl3 vs libssl1.1, etc.)
        set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
        if(ENABLE_STATIC_LINKING)
            # Static builds: rely on shlibdeps for only what is actually linked
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
        else()
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "libssl3 (>= 3.0.0), libjsoncpp25 | libjsoncpp24 | libjsoncpp1")
        endif()
        set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "systemd, libcap2-bin")
        set(CPACK_DEBIAN_PACKAGE_SUGGESTS "logrotate")
        set(CPACK_DEBIAN_PACKAGE_CONFLICTS "ntp")
        set(CPACK_DEBIAN_PACKAGE_PROVIDES "${PROJECT_NAME}")
        set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/simpledaemons/${PROJECT_NAME}")
        set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
        set(CPACK_DEBIAN_PACKAGE_LONG_DESCRIPTION "${PROJECT_DESCRIPTION}")

        # RPM package settings
        set(CPACK_RPM_PACKAGE_LICENSE "Apache-2.0")
        set(CPACK_RPM_PACKAGE_GROUP "Applications/System")
        set(CPACK_RPM_PACKAGE_URL "https://github.com/simpledaemons/${PROJECT_NAME}")
        set(CPACK_RPM_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
        if(ENABLE_STATIC_LINKING)
            set(CPACK_RPM_PACKAGE_REQUIRES "")
        else()
            set(CPACK_RPM_PACKAGE_REQUIRES "openssl-libs, jsoncpp")
        endif()
        set(CPACK_RPM_PACKAGE_SUGGESTS "systemd, libcap")
        set(CPACK_RPM_PACKAGE_CONFLICTS "ntp")
        set(CPACK_RPM_PACKAGE_PROVIDES "${PROJECT_NAME}")

        set(CPACK_PACKAGING_INSTALL_PREFIX "/")
        set(CPACK_SET_DESTDIR TRUE)
        set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/deb/postinst"
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/deb/prerm"
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/deb/postrm"
        )
        set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/postinstall.sh")
        set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/preuninstall.sh")
        set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE
            "${CMAKE_CURRENT_SOURCE_DIR}/packaging/linux/rpm/postuninstall.sh")
    endif()

    # Set package architecture and determine architecture string
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
        set(PACKAGE_ARCH "amd64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "aarch64")
        set(PACKAGE_ARCH "arm64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7l")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "armv7hl")
        set(PACKAGE_ARCH "armhf")
    else()
        set(PACKAGE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    # Determine distribution/platform identifier for package naming
    if(APPLE)
        # Detect Intel vs Apple Silicon for macOS
        # Try multiple methods to detect architecture
        set(PLATFORM_ID "macos-intel")  # Default fallback
        if(CMAKE_OSX_ARCHITECTURES)
            if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
                set(PLATFORM_ID "macos-apple")
            endif()
        elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
            set(PLATFORM_ID "macos-apple")
        else()
            # Fallback: use uname to detect architecture
            execute_process(
                COMMAND uname -m
                OUTPUT_VARIABLE UNAME_M
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if(UNAME_M MATCHES "arm64|aarch64")
                set(PLATFORM_ID "macos-apple")
            else()
                set(PLATFORM_ID "macos-intel")
            endif()
        endif()
        message(STATUS "Detected macOS platform: ${PLATFORM_ID}")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        set(PLATFORM_ID "freebsd")
    elseif(UNIX)
        # Try to detect Linux distribution for proper naming
        set(PLATFORM_ID "linux-generic")  # Default for RPM-based
        if(EXISTS "/etc/os-release")
            file(READ "/etc/os-release" OS_RELEASE)
            if(OS_RELEASE MATCHES "ID=([^\n]+)")
                string(REGEX REPLACE ".*ID=([^\n]+).*" "\\1" DISTRO_ID "${OS_RELEASE}")
                string(STRIP "${DISTRO_ID}" DISTRO_ID)
                string(TOLOWER "${DISTRO_ID}" DISTRO_ID)
                string(REPLACE "\"" "" DISTRO_ID "${DISTRO_ID}")

                # Map common distro IDs to package names
                if(DISTRO_ID MATCHES "debian|ubuntu")
                    set(PLATFORM_ID "linux-debian")
                endif()
            endif()
        endif()
    else()
        set(PLATFORM_ID "${PLATFORM_NAME}")
    endif()

    # Set package file names with standardized format: {project}-{version}-{platform}-{distro}-{arch}.{ext}
    # Format matches: simple-tftpd-0.2.1-linux-debian-amd64.deb
    if(APPLE)
        # macOS packages: simple-tftpd-0.2.1-macos-intel.dmg / .pkg
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_ID}")
        # Explicitly set generator-specific file names for macOS
        set(CPACK_DMG_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_ID}")
        set(CPACK_PRODUCTBUILD_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_ID}")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_ID}-${PACKAGE_ARCH}")
        set(CPACK_FREEBSD_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_ID}-${PACKAGE_ARCH}")
    elseif(UNIX)
        # Linux packages: simple-tftpd-0.2.1-linux-debian-amd64.deb / .rpm
        # CPack automatically adds architecture for DEB/RPM, so we set the base name
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_ID}-${PACKAGE_ARCH}")

        # Override DEB and RPM file names to match our format
        # CPack format: {CPACK_PACKAGE_FILE_NAME}_{CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb
        # We want: {name}-{version}-{platform}-{arch}.deb (architecture already in CPACK_PACKAGE_FILE_NAME)
        # So we remove architecture from CPACK_DEBIAN_FILE_NAME pattern
        set(CPACK_DEBIAN_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_ID}-${PACKAGE_ARCH}")
        set(CPACK_RPM_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_ID}-${PACKAGE_ARCH}")
    else()
        # Windows and others
        set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${PLATFORM_NAME}-${PACKAGE_ARCH}")
    endif()

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")

    # Component installation
    set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Application")
    set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "The ${PROJECT_NAME} executable and related files")

    set(CPACK_COMPONENT_DEVELOPMENT_DISPLAY_NAME "Development")
    set(CPACK_COMPONENT_DEVELOPMENT_DESCRIPTION "Header files for developing with ${PROJECT_NAME}")

    set(CPACK_COMPONENT_CONFIG_DISPLAY_NAME "Configuration")
    set(CPACK_COMPONENT_CONFIG_DESCRIPTION "Configuration files and examples")

    set(CPACK_COMPONENT_SERVICE_DISPLAY_NAME "Service")
    set(CPACK_COMPONENT_SERVICE_DESCRIPTION "System service files")

    # Set component dependencies
    set(CPACK_COMPONENT_DEVELOPMENT_DEPENDS applications)
    set(CPACK_COMPONENT_CONFIG_DEPENDS applications)
    set(CPACK_COMPONENT_SERVICE_DEPENDS applications)

    # Source package generation
    set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
    set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-src")
    set(CPACK_SOURCE_IGNORE_FILES
        "/build/"
        "/dist/"
        "/.git/"
        "/.gitignore"
        "/.vscode/"
        "/.vs/"
        "/CMakeCache.txt"
        "/CMakeFiles/"
        "/cmake_install.cmake"
        "/Makefile"
        "/install_manifest.txt"
        "/compile_commands.json"
        "/CTestTestfile.cmake"
        "/_CPack_Packages/"
        "/.DS_Store"
        "/Thumbs.db"
        "*.o"
        "*.a"
        "*.so"
        "*.dylib"
        "*.dll"
        "*.exe"
        "*.pdb"
        "*.ilk"
        "*.exp"
        "*.lib"
    )

    # Include CPack configuration
    include(CPack)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "  Platform: ${PLATFORM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Shared libraries: ${BUILD_SHARED_LIBS}")
message(STATUS "  Tests enabled: ${ENABLE_TESTS}")
message(STATUS "  Packaging enabled: ${ENABLE_PACKAGING}")
message(STATUS "  SSL support: ${ENABLE_SSL}")
message(STATUS "  JSON support: ${ENABLE_JSON}")
message(STATUS "")
