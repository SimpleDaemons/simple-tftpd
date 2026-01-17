# Memory Leak Detection Guide

**Date:** January 2025  
**Purpose:** Guide for running memory leak detection tests

---

## Overview

Memory leak detection is critical for production readiness. Simple TFTP Daemon supports two methods for detecting memory leaks:

1. **AddressSanitizer** - Works on Linux, macOS, and Windows
2. **Valgrind** - Linux only, more detailed analysis

---

## Quick Start

### Using the Automated Script

```bash
# Run memory leak detection tests
./scripts/run-memory-tests.sh
```

This script will:
- Build with AddressSanitizer enabled
- Run all tests with leak detection
- Generate detailed reports
- Optionally run Valgrind on Linux

---

## Method 1: AddressSanitizer

### Prerequisites

AddressSanitizer is built into modern compilers (GCC 4.8+, Clang 3.1+). No additional installation required.

### Manual Setup

```bash
# Clean build directory
rm -rf build
mkdir build
cd build

# Configure with AddressSanitizer
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_TESTS=ON \
    -DENABLE_SANITIZER=ON \
    -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g"

# Build
make -j$(nproc)

# Run tests
export ASAN_OPTIONS="detect_leaks=1:halt_on_error=0:abort_on_error=0:print_stats=1"
ctest --output-on-failure
```

### AddressSanitizer Options

- `detect_leaks=1` - Enable leak detection
- `halt_on_error=0` - Continue after errors (useful for test suites)
- `abort_on_error=0` - Don't abort on errors
- `print_stats=1` - Print statistics

### Interpreting Results

AddressSanitizer will report:
- **Direct leaks** - Memory allocated but not freed
- **Indirect leaks** - Memory reachable but not freed
- **Still reachable** - Memory still accessible (may be intentional)

Example output:
```
==12345==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 1024 bytes in 1 object(s) allocated from:
    #0 0x7f8b2c3d4e80 in malloc
    #1 0x55a1b2c3d4e8 in TftpConnection::allocateBuffer()
```

---

## Method 2: Valgrind (Linux Only)

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install valgrind

# CentOS/RHEL
sudo yum install valgrind

# Fedora
sudo dnf install valgrind
```

### Manual Setup

```bash
# Build without AddressSanitizer (for Valgrind)
rm -rf build
mkdir build
cd build

cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_TESTS=ON \
    -DENABLE_VALGRIND=ON \
    -DENABLE_SANITIZER=OFF

make -j$(nproc)

# Run unit tests with Valgrind
valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --error-exitcode=1 \
    --log-file=valgrind-unit-tests.log \
    ./bin/simple-tftpd-tests

# Run integration tests with Valgrind
valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --error-exitcode=1 \
    --log-file=valgrind-integration-tests.log \
    ./bin/simple-tftpd-integration-tests
```

### Valgrind Options

- `--leak-check=full` - Full leak detection
- `--show-leak-kinds=all` - Show all leak types
- `--track-origins=yes` - Track origin of uninitialized values
- `--error-exitcode=1` - Exit with error code if leaks found
- `--log-file=FILE` - Save output to file

### Interpreting Results

Valgrind categorizes leaks:
- **Definitely lost** - Memory definitely leaked (must fix)
- **Indirectly lost** - Memory reachable through leaked pointer
- **Possibly lost** - Memory may be leaked (investigate)
- **Still reachable** - Memory still accessible (may be intentional)

Example output:
```
==12345== 1,024 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2AB80: malloc (vg_replace_malloc.c:299)
==12345==    by 0x55A1B2C: TftpConnection::allocateBuffer() (connection.cpp:123)
```

---

## Fixing Memory Leaks

### Common Patterns

1. **Missing delete/free**
   ```cpp
   // BAD
   void* buffer = malloc(1024);
   // Missing: free(buffer);
   
   // GOOD
   void* buffer = malloc(1024);
   // ... use buffer ...
   free(buffer);
   ```

2. **Exception safety**
   ```cpp
   // BAD
   void* buffer = malloc(1024);
   if (error) return; // Leak!
   free(buffer);
   
   // GOOD
   void* buffer = malloc(1024);
   if (error) {
       free(buffer);
       return;
   }
   free(buffer);
   ```

3. **RAII (Resource Acquisition Is Initialization)**
   ```cpp
   // BAD
   void* buffer = malloc(1024);
   // ... use buffer ...
   free(buffer);
   
   // GOOD
   std::unique_ptr<void, decltype(&free)> buffer(malloc(1024), free);
   // Automatically freed when out of scope
   ```

---

## Continuous Integration

### GitHub Actions Example

```yaml
- name: Run Memory Leak Detection
  run: |
    cmake .. -DENABLE_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
    make -j$(nproc)
    export ASAN_OPTIONS="detect_leaks=1"
    ctest --output-on-failure
```

### Jenkins Example

```groovy
stage('Memory Leak Detection') {
    steps {
        sh '''
            cmake .. -DENABLE_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
            make -j$(nproc)
            export ASAN_OPTIONS="detect_leaks=1"
            ctest --output-on-failure
        '''
    }
}
```

---

## Best Practices

1. **Run regularly** - Include in CI/CD pipeline
2. **Fix immediately** - Don't let leaks accumulate
3. **Use both tools** - AddressSanitizer for speed, Valgrind for detail
4. **Review "still reachable"** - May indicate intentional caching
5. **Document intentional leaks** - If memory is intentionally kept, document it

---

## Troubleshooting

### AddressSanitizer Issues

**Problem:** Tests fail with AddressSanitizer errors
**Solution:** Check for:
- Uninitialized memory access
- Use-after-free
- Double-free
- Stack buffer overflow

**Problem:** Slow performance with AddressSanitizer
**Solution:** This is normal. AddressSanitizer adds ~2x overhead.

### Valgrind Issues

**Problem:** Valgrind reports false positives
**Solution:** Use suppression file:
```bash
valgrind --suppressions=valgrind.supp ./bin/simple-tftpd-tests
```

**Problem:** Valgrind is very slow
**Solution:** This is normal. Valgrind adds ~10-50x overhead. Use for detailed analysis only.

---

## References

- [AddressSanitizer Documentation](https://github.com/google/sanitizers/wiki/AddressSanitizer)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)
- [Memory Leak Detection Best Practices](https://www.owasp.org/index.php/Memory_leak)

---

*Last Updated: January 2025*
