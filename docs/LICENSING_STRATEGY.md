# Simple TFTP Daemon - Licensing Strategy

**Date:** January 2025  
**Version:** Dual-License Model (Apache 2.0 + BSL)

---

## Overview

Simple TFTP Daemon uses a dual-licensing model to provide both a free, open-source production-ready version and an enterprise version with advanced features:

- **Production Edition (Apache 2.0)**: Free, open-source, production-ready TFTP server
- **Enterprise Edition (BSL)**: Commercial license with advanced enterprise features

---

## License Models

### Production Edition - Apache 2.0 License

**Target Audience:** Individual users, small businesses, open-source projects, educational use

**License:** Apache License, Version 2.0

**Features Included:**
- ✅ Complete TFTP protocol implementation (RFC 1350, RFC 2347)
- ✅ Windowed transfers with sliding window
- ✅ TFTP options support (blksize, timeout, tsize, windowsize)
- ✅ Retransmission and duplicate packet detection
- ✅ Basic security features:
  - Path sanitization and directory traversal protection
  - Directory allowlists
  - File extension filtering
  - IP address filtering (client allowlists)
  - File size limits
  - Read/write permission toggles
  - Overwrite protection
- ✅ Multi-format configuration (JSON, YAML, INI)
- ✅ Configuration hot-reload (SIGHUP)
- ✅ Basic logging (file and console, multiple levels)
- ✅ Production monitoring:
  - Health checks
  - Basic metrics collection
  - Status APIs
- ✅ Cross-platform support (Linux, macOS, Windows)
- ✅ IPv6 support
- ✅ Build automation (CMake, Makefile)
- ✅ Package generation (DEB, RPM, DMG, PKG, MSI)
- ✅ Docker support
- ✅ Service integration (systemd, launchd, Windows Services)
- ✅ Comprehensive documentation

**Use Cases:**
- Small to medium deployments
- Development and testing environments
- Educational purposes
- Open-source projects
- Personal use

---

### Enterprise Edition - Business Source License (BSL)

**Target Audience:** Large enterprises, organizations requiring advanced features, commercial deployments

**License:** Business Source License (BSL) with conversion to Apache 2.0 after 4 years

**Additional Features (Beyond Production Edition):**
- 🔒 **Web Management Interface**
  - Web-based dashboard for server management
  - Real-time monitoring and statistics
  - Configuration editor with validation
  - File browser and management
  - User management interface

- 🔒 **REST API**
  - Complete REST API for programmatic control
  - Configuration management API
  - Statistics and monitoring API
  - Connection management API
  - Integration with external systems

- 🔒 **SNMP Integration**
  - SNMP agent for network management
  - MIB definitions for TFTP server metrics
  - Integration with network monitoring systems
  - Standard SNMP traps and notifications

- 🔒 **Advanced Authentication & Authorization**
  - User authentication system
  - Role-based access control (RBAC)
  - Per-user and per-client policies
  - Access control lists (ACLs)
  - Audit logging and compliance features

- 🔒 **Advanced Security Features**
  - Rate limiting per client/IP
  - DDoS protection and mitigation
  - Advanced threat detection
  - Security event logging
  - Compliance reporting

- 🔒 **Advanced Monitoring & Observability**
  - Structured JSON logging
  - Syslog integration
  - Advanced metrics and analytics
  - Performance profiling
  - Custom alerting and notifications
  - Integration with monitoring platforms (Prometheus, Grafana, etc.)

- 🔒 **High Availability & Clustering**
  - Multi-server coordination
  - Load balancing
  - Failover mechanisms
  - State synchronization
  - Distributed file serving

- 🔒 **Advanced Performance Features**
  - Async I/O operations
  - Advanced connection pooling
  - Memory pooling optimizations
  - Performance profiling tools
  - Advanced caching strategies

- 🔒 **Plugin Architecture**
  - Extensible plugin system
  - Custom authentication plugins
  - Custom storage backends
  - Custom protocol extensions
  - Third-party integrations

- 🔒 **Advanced Configuration**
  - Configuration templates
  - Environment variable support
  - Dynamic configuration updates via API
  - Configuration versioning
  - Configuration backup and restore

- 🔒 **Enterprise Support**
  - Priority support
  - SLA guarantees
  - Professional services
  - Custom development
  - Training and consulting

**Use Cases:**
- Large enterprise deployments
- High-availability requirements
- Compliance and audit requirements
- Integration with enterprise systems
- Commercial software products
- Managed service providers

---

## Feature Comparison Matrix

| Feature | Production (Apache 2.0) | Enterprise (BSL) |
|---------|-------------------------|------------------|
| **Core TFTP Protocol** | ✅ Full RFC 1350, RFC 2347 | ✅ Full RFC 1350, RFC 2347 |
| **Windowed Transfers** | ✅ Yes | ✅ Yes |
| **TFTP Options** | ✅ Yes | ✅ Yes |
| **Basic Security** | ✅ Yes | ✅ Yes |
| **Configuration Management** | ✅ JSON/YAML/INI | ✅ JSON/YAML/INI + Templates |
| **Hot Reload** | ✅ Yes | ✅ Yes |
| **Basic Logging** | ✅ Yes | ✅ Yes |
| **Health Checks** | ✅ Yes | ✅ Yes |
| **Basic Metrics** | ✅ Yes | ✅ Yes |
| **Cross-Platform** | ✅ Yes | ✅ Yes |
| **Docker Support** | ✅ Yes | ✅ Yes |
| **Web Management UI** | ❌ No | ✅ Yes |
| **REST API** | ❌ No | ✅ Yes |
| **SNMP Integration** | ❌ No | ✅ Yes |
| **User Authentication** | ❌ No | ✅ Yes |
| **RBAC/ACLs** | ❌ No | ✅ Yes |
| **Rate Limiting** | ❌ No | ✅ Yes |
| **DDoS Protection** | ❌ No | ✅ Yes |
| **Structured Logging** | ❌ No | ✅ Yes |
| **Syslog Integration** | ❌ No | ✅ Yes |
| **Clustering/HA** | ❌ No | ✅ Yes |
| **Async I/O** | ❌ No | ✅ Yes |
| **Plugin System** | ❌ No | ✅ Yes |
| **Enterprise Support** | ❌ No | ✅ Yes |

---

## Implementation Strategy

### Code Organization

1. **Core Library (Apache 2.0)**
   - All production features in main codebase
   - No enterprise code dependencies
   - Fully functional standalone server

2. **Enterprise Extensions (BSL)**
   - Separate modules/plugins for enterprise features
   - Optional compilation flags (`ENABLE_ENTERPRISE_FEATURES`)
   - Clear separation of concerns
   - Build-time feature flags

### Build Configuration

```cmake
# Production features (always enabled)
option(BUILD_PRODUCTION "Build production features" ON)

# Enterprise features (BSL licensed)
option(BUILD_ENTERPRISE "Build enterprise features (BSL license)" OFF)
option(ENABLE_WEB_UI "Enable web management interface" OFF)
option(ENABLE_REST_API "Enable REST API" OFF)
option(ENABLE_SNMP "Enable SNMP integration" OFF)
option(ENABLE_AUTH "Enable authentication system" OFF)
option(ENABLE_CLUSTERING "Enable clustering support" OFF)
option(ENABLE_PLUGINS "Enable plugin architecture" OFF)
```

### License Headers

**Production Code (Apache 2.0):**
```cpp
/*
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */
```

**Enterprise Code (BSL):**
```cpp
/*
 * Copyright 2024 SimpleDaemons
 *
 * This Source Code Form is subject to the terms of the Business Source License
 * (BSL) 1.1. You may not use this file except in compliance with the BSL.
 * See LICENSE-BSL.txt for details.
 *
 * For commercial licensing, contact: licensing@simpledaemons.com
 */
```

---

## Migration Path

### For Users

1. **Start with Production Edition**
   - Free, open-source, fully functional
   - All core features available
   - No restrictions on use

2. **Upgrade to Enterprise Edition**
   - When advanced features are needed
   - Commercial license required
   - Contact for licensing: licensing@simpledaemons.com

### For Developers

1. **Contribute to Production Edition**
   - All contributions under Apache 2.0
   - Open-source development model
   - Community-driven improvements

2. **Enterprise Development**
   - Separate enterprise codebase
   - BSL licensed
   - Commercial development model

---

## Versioning Strategy

### Production Edition
- **Version Format:** `v0.3.0`, `v0.4.0`, `v1.0.0`, etc.
- **Release Cycle:** Regular releases with production features
- **License:** Always Apache 2.0

### Enterprise Edition
- **Version Format:** `v0.3.0-enterprise`, `v0.4.0-enterprise`, etc.
- **Release Cycle:** Aligned with production releases + enterprise features
- **License:** BSL (converts to Apache 2.0 after 4 years)

---

## Roadmap Alignment

### Production Edition Roadmap (Apache 2.0)

**v0.3.0 (Production Ready)**
- ✅ Core TFTP protocol
- ✅ Basic security
- ✅ Configuration management
- ✅ Production monitoring
- ✅ Comprehensive testing
- ✅ Documentation

**v0.4.0 (Enhanced Production)**
- Performance optimizations
- Enhanced logging
- Additional security features
- Improved monitoring

**v1.0.0 (Stable Production)**
- Long-term support
- Stability focus
- Performance improvements

### Enterprise Edition Roadmap (BSL)

**v0.4.0-enterprise (First Enterprise Release)**
- Web management interface
- REST API
- SNMP integration
- Advanced authentication

**v0.5.0-enterprise**
- Clustering support
- High availability
- Plugin architecture

**v1.0.0-enterprise**
- Complete enterprise feature set
- Full enterprise support

---

## Legal Considerations

### Apache 2.0 License
- Permissive open-source license
- Allows commercial use
- Allows modification and distribution
- Patent grant included

### Business Source License (BSL)
- Source-available license
- Allows viewing and modification of source code
- Commercial use requires license
- Converts to Apache 2.0 after 4 years (or specified period)
- Protects commercial interests while maintaining transparency

### Dual Licensing
- Clear separation of codebases
- No license conflicts
- Production code remains fully open-source
- Enterprise code is source-available with commercial restrictions

---

## Support Model

### Production Edition Support
- Community support (GitHub Issues, Discussions)
- Community documentation
- Open-source development model
- No SLA guarantees

### Enterprise Edition Support
- Priority support channels
- SLA guarantees
- Professional services
- Custom development
- Training and consulting
- Dedicated support team

---

## FAQ

### Q: Can I use Production Edition commercially?
**A:** Yes! Apache 2.0 allows commercial use without restrictions.

### Q: What happens to Enterprise features after 4 years?
**A:** BSL converts to Apache 2.0, making enterprise features available in Production Edition.

### Q: Can I contribute to Enterprise Edition?
**A:** Enterprise Edition contributions require BSL licensing. Contact us for details.

### Q: How do I get Enterprise Edition?
**A:** Contact licensing@simpledaemons.com for commercial licensing information.

### Q: Can I build Enterprise Edition from source?
**A:** Yes, source code is available under BSL. Commercial use requires a license.

### Q: Will Production Edition always be free?
**A:** Yes, Production Edition will always be free and open-source under Apache 2.0.

---

## Contact

For licensing inquiries:
- **Email:** licensing@simpledaemons.com
- **Website:** https://simpledaemons.com
- **GitHub:** https://github.com/SimpleDaemons/simple-tftpd

---

*Last Updated: January 2025*  
*This document is subject to change as the licensing strategy evolves.*
