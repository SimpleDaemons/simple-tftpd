# Build Guide

Complete guide to building Simple TFTP Daemon for different versions and platforms.

## Quick Start

```bash
# Clone repository
git clone https://github.com/SimpleDaemons/simple-tftpd.git
cd simple-tftpd

# Build Production version
mkdir build && cd build
cmake -DBUILD_VERSION=production ..
make

# Run
./simple-tftpd --help
```

---

## Build Commands Reference

### Basic Build

```bash
# Configure
cmake ..

# Build
make

# Install (requires root)
sudo make install
```

### Version-Specific Builds

#### Production Version
```bash
cmake -DBUILD_VERSION=production ..
make
```

#### Enterprise Version
```bash
cmake -DBUILD_VERSION=enterprise ..
make
```

#### Datacenter Version
```bash
cmake -DBUILD_VERSION=datacenter ..
make
```

### Build Types

#### Debug Build
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

#### Release Build
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Build Options

#### Disable Tests
```bash
cmake -DENABLE_TESTS=OFF ..
```

#### Static Linking
```bash
cmake -DENABLE_STATIC_LINKING=ON ..
```

---

## Platform-Specific Builds

### Linux

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake git libjsoncpp-dev pkg-config

mkdir build && cd build
cmake -DBUILD_VERSION=production ..
make -j$(nproc)
```

#### CentOS/RHEL
```bash
sudo yum groupinstall "Development Tools"
sudo yum install cmake git jsoncpp-devel pkgconfig

mkdir build && cd build
cmake -DBUILD_VERSION=production ..
make -j$(nproc)
```

### macOS

```bash
brew install cmake jsoncpp pkg-config

mkdir build && cd build
cmake -DBUILD_VERSION=production \
      -DJSONCPP_ROOT_DIR=/usr/local \
      ..
make -j$(sysctl -n hw.ncpu)
```

### Windows

#### Visual Studio
```powershell
# Install dependencies via vcpkg
vcpkg install jsoncpp:x64-windows

# Configure
mkdir build
cd build
cmake -DBUILD_VERSION=production `
      -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
      ..

# Build
cmake --build . --config Release
```

---

## Testing

### Run Tests

```bash
# Build with tests enabled (default)
cmake -DENABLE_TESTS=ON ..
make

# Run all tests
make test

# Or use CTest
ctest

# Run with verbose output
ctest --verbose
```

---

## Troubleshooting

### CMake Not Finding Dependencies

**Problem:** CMake can't find jsoncpp

**Solution:**
```bash
# Set PKG_CONFIG_PATH
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

# Or specify paths
cmake -DJSONCPP_ROOT_DIR=/usr/local ..
```

### Compilation Errors

**Problem:** C++17 features not recognized

**Solution:**
```bash
# Check compiler version
g++ --version  # Should be 7+ or clang 8+

# Force C++17
cmake -DCMAKE_CXX_STANDARD=17 ..
```

---

**Last Updated:** December 2024
