#!/bin/bash
# Simple TFTP Daemon macOS Build Script
# Copyright 2024 SimpleDaemons
# Licensed under Apache License 2.0

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Project information
PROJECT_NAME="simple-tftpd"
VERSION="0.1.0"
BUILD_DIR="build"
BIN_DIR="${BUILD_DIR}/bin"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check macOS version
check_macos_version() {
    local macos_version=$(sw_vers -productVersion)
    local major_version=$(echo "$macos_version" | cut -d. -f1)
    local minor_version=$(echo "$macos_version" | cut -d. -f2)
    
    print_status "Detected macOS version: $macos_version"
    
    if [ "$major_version" -lt 12 ]; then
        print_error "macOS 12.0 (Monterey) or higher is required"
        exit 1
    fi
    
    print_success "macOS version check passed"
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check for Homebrew
    if ! command_exists brew; then
        print_error "Homebrew is not installed. Please install it first:"
        echo "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    
    # Check for CMake
    if ! command_exists cmake; then
        print_warning "CMake not found. Installing via Homebrew..."
        brew install cmake
    fi
    
    # Check for jsoncpp
    if ! brew list jsoncpp >/dev/null 2>&1; then
        print_warning "jsoncpp not found. Installing via Homebrew..."
        brew install jsoncpp
    fi
    
    # Check for pkg-config
    if ! command_exists pkg-config; then
        print_warning "pkg-config not found. Installing via Homebrew..."
        brew install pkg-config
    fi
    
    print_success "All dependencies are available"
}

# Function to setup build environment
setup_build_env() {
    print_status "Setting up build environment..."
    
    # Create build directory
    mkdir -p "$BUILD_DIR"
    
    # Set environment variables
    export CMAKE_PREFIX_PATH="$(brew --prefix)"
    export PKG_CONFIG_PATH="$(brew --prefix)/lib/pkgconfig"
    
    print_success "Build environment setup complete"
}

# Function to build project
build_project() {
    print_status "Building $PROJECT_NAME v$VERSION..."
    
    cd "$BUILD_DIR"
    
    # Configure with CMake
    print_status "Configuring with CMake..."
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="12.0" \
        -DBUILD_TESTS=ON \
        -DBUILD_EXAMPLES=OFF \
        -DENABLE_LOGGING=ON \
        -DENABLE_IPV6=ON
    
    # Build
    print_status "Building..."
    make -j$(sysctl -n hw.ncpu)
    
    cd ..
    
    print_success "Build completed successfully"
}

# Function to run tests
run_tests() {
    print_status "Running tests..."
    
    if [ -f "$BIN_DIR/simple-tftpd-tests" ]; then
        cd "$BIN_DIR"
        ./simple-tftpd-tests
        cd ../..
        print_success "Tests completed"
    else
        print_warning "Test executable not found. Skipping tests."
    fi
}

# Function to install project
install_project() {
    print_status "Installing $PROJECT_NAME..."
    
    cd "$BUILD_DIR"
    make install
    cd ..
    
    print_success "Installation completed"
}

# Function to create package
create_package() {
    print_status "Creating package..."
    
    cd "$BUILD_DIR"
    make package
    cd ..
    
    print_success "Package created successfully"
}

# Function to show help
show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help      Show this help message"
    echo "  -c, --clean     Clean build directory before building"
    echo "  -t, --test      Run tests after building"
    echo "  -i, --install   Install after building"
    echo "  -p, --package   Create package after building"
    echo "  -a, --all       Clean, build, test, install, and package"
    echo ""
    echo "Examples:"
    echo "  $0              # Build only"
    echo "  $0 --test       # Build and run tests"
    echo "  $0 --all        # Full build process"
}

# Main function
main() {
    local clean_build=false
    local run_tests_flag=false
    local install_flag=false
    local package_flag=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                clean_build=true
                shift
                ;;
            -t|--test)
                run_tests_flag=true
                shift
                ;;
            -i|--install)
                install_flag=true
                shift
                ;;
            -p|--package)
                package_flag=true
                shift
                ;;
            -a|--all)
                clean_build=true
                run_tests_flag=true
                install_flag=true
                package_flag=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    print_status "Starting build process for $PROJECT_NAME v$VERSION"
    
    # Check system requirements
    check_macos_version
    check_dependencies
    
    # Clean build directory if requested
    if [ "$clean_build" = true ]; then
        print_status "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    # Setup build environment
    setup_build_env
    
    # Build project
    build_project
    
    # Run tests if requested
    if [ "$run_tests_flag" = true ]; then
        run_tests
    fi
    
    # Install if requested
    if [ "$install_flag" = true ]; then
        install_project
    fi
    
    # Create package if requested
    if [ "$package_flag" = true ]; then
        create_package
    fi
    
    print_success "Build process completed successfully!"
    
    # Show build results
    if [ -f "$BIN_DIR/$PROJECT_NAME" ]; then
        print_status "Binary location: $BIN_DIR/$PROJECT_NAME"
        print_status "Binary size: $(ls -lh "$BIN_DIR/$PROJECT_NAME" | awk '{print $5}')"
    fi
}

# Run main function
main "$@"
