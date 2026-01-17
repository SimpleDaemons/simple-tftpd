#!/bin/bash
# Memory Leak Detection Test Runner
# Copyright 2024 SimpleDaemons
# Licensed under Apache License 2.0

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

echo "=========================================="
echo "Memory Leak Detection Test Runner"
echo "=========================================="
echo ""

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Please run 'make build' first."
    exit 1
fi

cd "$BUILD_DIR"

# Detect platform
UNAME_S=$(uname -s)
PLATFORM="unknown"

if [[ "$UNAME_S" == "Linux" ]]; then
    PLATFORM="linux"
elif [[ "$UNAME_S" == "Darwin" ]]; then
    PLATFORM="macos"
elif [[ "$UNAME_S" == MINGW* ]] || [[ "$UNAME_S" == MSYS* ]] || [[ "$UNAME_S" == CYGWIN* ]]; then
    PLATFORM="windows"
fi

echo "Platform: $PLATFORM"
echo ""

# Method 1: AddressSanitizer (works on Linux, macOS, Windows)
echo "=========================================="
echo "Method 1: AddressSanitizer"
echo "=========================================="
echo ""

if [ -d "$BUILD_DIR" ]; then
    echo "Configuring build with AddressSanitizer..."
    cd "$PROJECT_ROOT"
    
    # Clean and reconfigure with AddressSanitizer
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Debug \
        -DENABLE_TESTS=ON \
        -DENABLE_SANITIZER=ON \
        -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g"
    
    echo ""
    echo "Building with AddressSanitizer..."
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    echo ""
    echo "Running tests with AddressSanitizer..."
    echo "Note: AddressSanitizer will report memory leaks if any are found."
    echo ""
    
    # Set AddressSanitizer options
    export ASAN_OPTIONS="detect_leaks=1:halt_on_error=0:abort_on_error=0:print_stats=1"
    
    # Run tests
    if ctest --output-on-failure 2>&1 | tee asan-test-results.log; then
        echo ""
        echo "✅ AddressSanitizer tests completed successfully!"
        echo "   Results saved to: $BUILD_DIR/asan-test-results.log"
    else
        echo ""
        echo "⚠️  AddressSanitizer tests found issues. Check asan-test-results.log"
        exit 1
    fi
else
    echo "Build directory not found. Please run 'make build' first."
    exit 1
fi

echo ""
echo "=========================================="
echo "Method 2: Valgrind (Linux only)"
echo "=========================================="
echo ""

if [[ "$PLATFORM" == "linux" ]]; then
    if command -v valgrind &> /dev/null; then
        echo "Valgrind found. Running tests with Valgrind..."
        echo ""
        
        # Rebuild without AddressSanitizer for Valgrind
        cd "$PROJECT_ROOT"
        rm -rf "$BUILD_DIR"
        mkdir -p "$BUILD_DIR"
        cd "$BUILD_DIR"
        
        cmake .. \
            -DCMAKE_BUILD_TYPE=Debug \
            -DENABLE_TESTS=ON \
            -DENABLE_VALGRIND=ON \
            -DENABLE_SANITIZER=OFF
        
        make -j$(nproc 2>/dev/null || echo 4)
        
        echo ""
        echo "Running unit tests with Valgrind..."
        valgrind \
            --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --error-exitcode=1 \
            --log-file=valgrind-unit-tests.log \
            ./bin/simple-tftpd-tests 2>&1 | tee valgrind-unit-output.log
        
        if [ $? -eq 0 ]; then
            echo ""
            echo "✅ Valgrind unit tests passed!"
            echo "   Full report: $BUILD_DIR/valgrind-unit-tests.log"
        else
            echo ""
            echo "⚠️  Valgrind found memory leaks. Check valgrind-unit-tests.log"
        fi
        
        echo ""
        echo "Running integration tests with Valgrind..."
        valgrind \
            --leak-check=full \
            --show-leak-kinds=all \
            --track-origins=yes \
            --error-exitcode=1 \
            --log-file=valgrind-integration-tests.log \
            ./bin/simple-tftpd-integration-tests 2>&1 | tee valgrind-integration-output.log
        
        if [ $? -eq 0 ]; then
            echo ""
            echo "✅ Valgrind integration tests passed!"
            echo "   Full report: $BUILD_DIR/valgrind-integration-tests.log"
        else
            echo ""
            echo "⚠️  Valgrind found memory leaks. Check valgrind-integration-tests.log"
        fi
    else
        echo "Valgrind not found. Install with: sudo apt-get install valgrind"
        echo "   (or: sudo yum install valgrind)"
    fi
else
    echo "Valgrind is only available on Linux. Use AddressSanitizer on $PLATFORM."
fi

echo ""
echo "=========================================="
echo "Summary"
echo "=========================================="
echo ""
echo "Memory leak detection completed!"
echo ""
echo "Results:"
if [ -f "$BUILD_DIR/asan-test-results.log" ]; then
    echo "  - AddressSanitizer: $BUILD_DIR/asan-test-results.log"
fi
if [ -f "$BUILD_DIR/valgrind-unit-tests.log" ]; then
    echo "  - Valgrind Unit Tests: $BUILD_DIR/valgrind-unit-tests.log"
fi
if [ -f "$BUILD_DIR/valgrind-integration-tests.log" ]; then
    echo "  - Valgrind Integration Tests: $BUILD_DIR/valgrind-integration-tests.log"
fi
echo ""
echo "Next steps:"
echo "  1. Review the log files for any memory leaks"
echo "  2. Fix any identified leaks"
echo "  3. Re-run this script to verify fixes"
echo ""
