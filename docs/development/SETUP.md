# Developer Setup Guide

Step-by-step guide to set up your development environment for Simple TFTP Daemon.

## Prerequisites Checklist

- [ ] C++ compiler (GCC 7+ or Clang 8+)
- [ ] CMake 3.16+
- [ ] Git
- [ ] jsoncpp development libraries
- [ ] pkg-config

---

## Step 1: Install System Dependencies

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    libjsoncpp-dev \
    pkg-config
```

### Linux (CentOS/RHEL/Fedora)

```bash
sudo yum groupinstall -y "Development Tools"
sudo yum install -y \
    cmake \
    git \
    jsoncpp-devel \
    pkgconfig
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install \
    cmake \
    jsoncpp \
    pkg-config
```

### Windows

1. Install **Visual Studio 2019 or later** with C++ workload
2. Install **CMake** from [cmake.org](https://cmake.org/download/)
3. Install **vcpkg** and dependencies:
   ```powershell
   vcpkg install jsoncpp:x64-windows
   ```

---

## Step 2: Clone Repository

```bash
git clone https://github.com/SimpleDaemons/simple-tftpd.git
cd simple-tftpd
```

---

## Step 3: Verify Dependencies

### Check Compiler

```bash
g++ --version
# Should show: g++ (GCC) 7.0.0 or higher
```

### Check CMake

```bash
cmake --version
# Should show: cmake version 3.16.0 or higher
```

### Check Libraries

```bash
pkg-config --modversion jsoncpp
# Should show: 1.7.0 or higher
```

---

## Step 4: Initial Build

### Create Build Directory

```bash
mkdir build
cd build
```

### Configure Build

```bash
# Production version (recommended for development)
cmake -DBUILD_VERSION=production \
      -DCMAKE_BUILD_TYPE=Debug \
      -DENABLE_TESTS=ON \
      ..
```

### Build

```bash
# Build all targets
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS
```

### Verify Build

```bash
# Check if binary was created
ls -lh simple-tftpd

# Test the binary
./simple-tftpd --version
./simple-tftpd --help
```

---

## Step 5: Run Tests

```bash
# Run all tests
make test

# Or use CTest
ctest

# Run with verbose output
ctest --verbose
```

---

## Step 6: Development Workflow

### Daily Development

1. **Update your branch:**
   ```bash
   git pull origin main
   ```

2. **Create feature branch:**
   ```bash
   git checkout -b feature/your-feature
   ```

3. **Make changes and test:**
   ```bash
   cd build
   make -j$(nproc)
   make test
   ```

4. **Commit changes:**
   ```bash
   git add .
   git commit -m "feat: Add your feature"
   ```

---

## Troubleshooting Setup

### Issue: CMake can't find jsoncpp

**Solution:**
```bash
# Linux
sudo apt install libjsoncpp-dev

# macOS
brew install jsoncpp
export PKG_CONFIG_PATH=/usr/local/opt/jsoncpp/lib/pkgconfig:$PKG_CONFIG_PATH

# Or specify path
cmake -DJSONCPP_ROOT_DIR=/usr/local ..
```

### Issue: Permission denied errors

**Solution:**
```bash
# Make scripts executable
chmod +x scripts/*.sh

# For running TFTP server, use sudo
sudo ./simple-tftpd -c /etc/simple-tftpd/simple-tftpd.conf
```

---

## Next Steps

- Read [Developer README](README.md) for development guidelines
- Check [Build Guide](BUILD_GUIDE.md) for build commands
- Review code structure
- Join development discussions

---

**Last Updated:** December 2024
