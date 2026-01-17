# Feature Licensing Matrix

**Date:** January 2025  
**Purpose:** Detailed breakdown of which features belong to which license tier

---

## Production Edition (Apache 2.0) - Free & Open Source

### ✅ Core Protocol Features
- [x] TFTP Protocol (RFC 1350) - Complete implementation
- [x] TFTP Options (RFC 2347) - blksize, timeout, tsize, windowsize
- [x] Windowed Transfers - Sliding window implementation
- [x] Retransmission Logic - Automatic retry with configurable retries
- [x] Duplicate Packet Detection - Prevents duplicate processing
- [x] Transfer Modes - netascii, octet, mail
- [x] OACK Generation - Option acknowledgment packets
- [x] IPv6 Support - Optional IPv6 support

### ✅ Security Features (Basic)
- [x] Path Sanitization - Directory traversal protection
- [x] Directory Allowlists - Access control for directories
- [x] File Extension Filtering - Extension allowlists
- [x] IP Address Filtering - Client allowlists
- [x] File Size Limits - Maximum file size enforcement
- [x] Overwrite Protection - File overwrite control
- [x] Read/Write Toggles - Permission control
- [x] Transfer Logging - Basic transfer logging

### ✅ Configuration Management
- [x] JSON Configuration - Full JSON support
- [x] YAML Configuration - Full YAML support
- [x] INI Configuration - Full INI support
- [x] Configuration Validation - Comprehensive validation
- [x] Configuration Serialization - Save/load configuration
- [x] Hot Reload - SIGHUP signal support
- [x] Command-line Overrides - Runtime configuration

### ✅ Logging & Monitoring (Basic)
- [x] File Logging - Log to file
- [x] Console Logging - Log to console
- [x] Log Levels - DEBUG, INFO, WARNING, ERROR, FATAL
- [x] Health Checks - Basic health check API
- [x] Basic Metrics - Transfer statistics, connection stats
- [x] Status APIs - getMetrics(), getHealthCheckJson()

### ✅ Platform & Deployment
- [x] Cross-Platform Support - Linux, macOS, Windows
- [x] CMake Build System - Modern build system
- [x] Makefile Support - Traditional build system
- [x] Package Generation - DEB, RPM, DMG, PKG, MSI
- [x] Docker Support - Dockerfile and docker-compose
- [x] Service Integration - systemd, launchd, Windows Services
- [x] Build Automation - Ansible playbooks

### ✅ Documentation
- [x] User Guides - Complete user documentation
- [x] Configuration Guide - Configuration reference
- [x] Production Deployment Guide - Deployment instructions
- [x] Troubleshooting Guide - Common issues and solutions
- [x] API Documentation - Header file documentation
- [x] Examples - Usage examples

---

## Enterprise Edition (BSL) - Commercial License

### 🔒 Web Management Interface
- [ ] Web Dashboard - Real-time monitoring and management
- [ ] Configuration Editor - Web-based configuration management
- [ ] File Browser - Web-based file management
- [ ] Statistics Dashboard - Visual statistics and metrics
- [ ] User Management UI - Web-based user administration
- [ ] Connection Management UI - Web-based connection monitoring

### 🔒 REST API
- [ ] REST API Server - HTTP/HTTPS API endpoint
- [ ] Configuration API - Programmatic configuration management
- [ ] Statistics API - Programmatic metrics access
- [ ] Connection API - Programmatic connection management
- [ ] Authentication API - API authentication and authorization
- [ ] Webhook Support - Event notifications via webhooks

### 🔒 SNMP Integration
- [ ] SNMP Agent - Full SNMP agent implementation
- [ ] MIB Definitions - Complete MIB for TFTP server
- [ ] SNMP Traps - Event notifications via SNMP
- [ ] Network Management Integration - Integration with NMS systems
- [ ] SNMP v1/v2c/v3 Support - Multiple SNMP versions

### 🔒 Advanced Authentication & Authorization
- [ ] User Authentication System - Username/password authentication
- [ ] Role-Based Access Control (RBAC) - Role-based permissions
- [ ] Access Control Lists (ACLs) - Fine-grained access control
- [ ] Per-User Policies - User-specific configurations
- [ ] Per-Client Policies - Client-specific configurations
- [ ] LDAP/Active Directory Integration - Enterprise directory integration
- [ ] OAuth/OIDC Support - Modern authentication protocols
- [ ] Multi-Factor Authentication (MFA) - Enhanced security
- [ ] Session Management - User session handling

### 🔒 Advanced Security Features
- [ ] Rate Limiting - Per-client/IP rate limiting
- [ ] DDoS Protection - Distributed denial-of-service protection
- [ ] Advanced Threat Detection - Anomaly detection
- [ ] Security Event Logging - Comprehensive security audit logs
- [ ] Compliance Reporting - Audit and compliance reports
- [ ] IP Reputation Checking - Integration with threat intelligence
- [ ] Geo-blocking - Geographic access control
- [ ] Advanced Firewall Rules - Complex firewall rule engine

### 🔒 Advanced Monitoring & Observability
- [ ] Structured JSON Logging - Machine-readable logs
- [ ] Syslog Integration - Integration with syslog servers
- [ ] Advanced Metrics - Extended metrics collection
- [ ] Performance Profiling - Detailed performance analysis
- [ ] Custom Alerting - Configurable alerts and notifications
- [ ] Prometheus Integration - Prometheus metrics export
- [ ] Grafana Integration - Grafana dashboard support
- [ ] Log Aggregation - Centralized log collection
- [ ] Distributed Tracing - Request tracing across services

### 🔒 High Availability & Clustering
- [ ] Multi-Server Coordination - Server cluster management
- [ ] Load Balancing - Request distribution across servers
- [ ] Failover Mechanisms - Automatic failover
- [ ] State Synchronization - Shared state across cluster
- [ ] Distributed File Serving - Files served from cluster
- [ ] Health Check Coordination - Cluster health monitoring
- [ ] Configuration Synchronization - Config sync across cluster

### 🔒 Advanced Performance Features
- [ ] Async I/O Operations - Asynchronous file I/O
- [ ] Advanced Connection Pooling - Optimized connection management
- [ ] Memory Pooling - Custom memory allocators
- [ ] Performance Profiling Tools - Built-in profiling
- [ ] Advanced Caching - Multi-level caching strategies
- [ ] Zero-Copy Transfers - Optimized data transfer
- [ ] CPU Affinity - Thread/process CPU binding

### 🔒 Plugin Architecture
- [ ] Plugin System - Extensible plugin framework
- [ ] Authentication Plugins - Custom authentication backends
- [ ] Storage Plugins - Custom storage backends
- [ ] Protocol Plugins - Custom protocol extensions
- [ ] Integration Plugins - Third-party integrations
- [ ] Plugin API - Plugin development SDK
- [ ] Plugin Marketplace - Plugin distribution platform

### 🔒 Advanced Configuration
- [ ] Configuration Templates - Pre-built configuration templates
- [ ] Environment Variable Support - Environment-based config
- [ ] Dynamic Configuration Updates - Runtime config changes via API
- [ ] Configuration Versioning - Config version control
- [ ] Configuration Backup/Restore - Config management
- [ ] Configuration Validation Rules - Advanced validation
- [ ] Multi-Environment Support - Dev/staging/prod configs

### 🔒 Enterprise Support
- [ ] Priority Support - Dedicated support channels
- [ ] SLA Guarantees - Service level agreements
- [ ] Professional Services - Custom development
- [ ] Training & Consulting - Expert guidance
- [ ] Custom Development - Tailored solutions
- [ ] On-Site Support - On-premises assistance

---

## Feature Migration Timeline

### BSL to Apache 2.0 Conversion

Enterprise features convert to Apache 2.0 after 4 years from initial release:

- **v0.4.0-enterprise features** → Apache 2.0 in **2029**
- **v0.5.0-enterprise features** → Apache 2.0 in **2030**
- **v1.0.0-enterprise features** → Apache 2.0 in **2031**

This ensures:
- Long-term open-source availability
- Fair commercial protection period
- Community benefit over time

---

## Build Flags Reference

### Production Edition (Default)
```bash
cmake -DBUILD_PRODUCTION=ON -DBUILD_ENTERPRISE=OFF ..
```

### Enterprise Edition
```bash
cmake -DBUILD_PRODUCTION=ON -DBUILD_ENTERPRISE=ON \
      -DENABLE_WEB_UI=ON \
      -DENABLE_REST_API=ON \
      -DENABLE_SNMP=ON \
      -DENABLE_AUTH=ON \
      -DENABLE_CLUSTERING=ON \
      -DENABLE_PLUGINS=ON ..
```

### Selective Enterprise Features
```bash
# Only enable specific enterprise features
cmake -DBUILD_PRODUCTION=ON -DBUILD_ENTERPRISE=ON \
      -DENABLE_WEB_UI=ON \
      -DENABLE_REST_API=ON \
      -DENABLE_SNMP=OFF \
      -DENABLE_AUTH=OFF \
      -DENABLE_CLUSTERING=OFF \
      -DENABLE_PLUGINS=OFF ..
```

---

## Code Organization

### Directory Structure
```
simple-tftpd/
├── src/
│   ├── production/          # Apache 2.0 licensed
│   │   ├── core/
│   │   ├── config/
│   │   └── utils/
│   └── enterprise/          # BSL licensed
│       ├── web/
│       ├── api/
│       ├── snmp/
│       ├── auth/
│       └── clustering/
├── include/
│   ├── simple-tftpd/        # Apache 2.0 headers
│   └── simple-tftpd-enterprise/  # BSL headers
└── docs/
    ├── production/          # Production docs
    └── enterprise/         # Enterprise docs
```

---

*Last Updated: January 2025*
