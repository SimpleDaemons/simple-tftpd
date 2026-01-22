# Source Code Structure Proposal

## Current Structure

```
simple-tftpd/
├── include/simple-tftpd/
│   ├── core/          # Core TFTP functionality
│   ├── config/        # Configuration
│   └── utils/         # Utilities
├── src/simple-tftpd/  # Implementation (mirrors include)
└── CMakeLists.txt     # Single build configuration
```

**Issues:**
- All features compiled into single binary
- No clear separation between Production/Enterprise/Datacenter
- Difficult to build version-specific binaries
- All code included regardless of version

## Proposed Structure

### Modular Directory Structure (Recommended)

```
simple-tftpd/
├── src/
│   ├── core/                    # Shared core (Production base)
│   │   ├── tftp/
│   │   │   ├── server.cpp
│   │   │   ├── connection.cpp
│   │   │   └── packet.cpp
│   │   ├── config/
│   │   └── utils/
│   ├── production/              # Production-specific features
│   │   ├── security/           # Basic security
│   │   └── features/            # Production-only features
│   ├── enterprise/              # Enterprise-specific features
│   │   ├── ha/                  # High availability
│   │   ├── management/          # Web UI, REST API
│   │   ├── integrations/       # SNMP, etc.
│   │   └── security/            # Advanced security
│   └── datacenter/              # Datacenter-specific features
│       ├── cluster/             # Clustering
│       ├── multi-site/          # Multi-site sync
│       ├── plugins/             # Plugin system
│       └── cloud/                # Cloud integrations
├── include/
│   └── simple-tftpd/
│       ├── core/                # Core headers
│       ├── production/          # Production headers
│       ├── enterprise/          # Enterprise headers
│       └── datacenter/          # Datacenter headers
├── main/
│   ├── production.cpp           # Production main
│   ├── enterprise.cpp          # Enterprise main
│   └── datacenter.cpp          # Datacenter main
└── CMakeLists.txt               # Version-aware build
```

**Benefits:**
- Clear separation of concerns
- Easy to build version-specific binaries
- Code reuse through shared core
- Maintainable and scalable

## Implementation Plan

1. **Phase 1: Reorganize Core**
   - Move shared code to `src/core/`
   - Update includes
   - Ensure Production builds

2. **Phase 2: Extract Production Features**
   - Move Production-specific code to `src/production/`
   - Update CMakeLists.txt for BUILD_VERSION=production

3. **Phase 3: Prepare Enterprise/Datacenter**
   - Create directory structure
   - Add placeholder files
   - Update CMakeLists.txt

## Build Commands

```bash
# Build Production
cmake -DBUILD_VERSION=production ..
make

# Build Enterprise
cmake -DBUILD_VERSION=enterprise ..
make

# Build Datacenter
cmake -DBUILD_VERSION=datacenter ..
make
```
