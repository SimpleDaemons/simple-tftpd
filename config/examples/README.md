# Configuration Examples

This directory contains example configuration files for the Simple TFTP Daemon, organized by complexity and use case.

## Directory Structure

```
config/examples/
├── simple/           # Basic configurations for common use cases
├── advanced/         # Advanced configurations with additional features
├── production/       # Production-ready configurations
└── README.md         # This file
```

## Configuration Formats

### JSON Configuration (Current)
- **File Extension**: `.conf` or `.json`
- **Status**: ✅ Fully supported
- **Use Case**: Primary configuration format

### YAML Configuration (Planned)
- **File Extension**: `.yaml` or `.yml`
- **Status**: 🔄 Planned for future release
- **Use Case**: Alternative configuration format for easier readability

## Simple Configurations

### `basic_server.conf.example`
Minimal configuration for basic TFTP server setup. Includes only essential settings.

**Best for**: Quick setup, testing, development environments

### `read_only.conf.example`
Secure read-only configuration for serving files without write access.

**Best for**: Public file servers, firmware distribution, ISO repositories

### `development.conf.example`
Development-friendly configuration with debugging enabled and local paths.

**Best for**: Development, testing, local development environments

## Advanced Configurations

### `secure_server.conf.example`
Advanced security features including access control, file filtering, and network restrictions.

**Best for**: Production environments requiring high security

### `high_performance.conf.example`
Performance-optimized configuration with large block sizes and connection pooling.

**Best for**: High-throughput scenarios, large file transfers

### `multi_tenant.conf.example`
Multi-tenant configuration supporting isolated environments for different user groups.

**Best for**: Shared infrastructure, hosting providers, enterprise environments

## Production Configurations

Production configurations will be added here with enterprise-grade features including:
- High availability configurations
- Load balancing setups
- Monitoring and alerting integration
- Backup and disaster recovery

## Usage

1. **Copy the appropriate example**:
   ```bash
   cp config/examples/simple/basic_server.conf.example /etc/simple-tftpd/simple-tftpd.conf
   ```

2. **Edit the configuration** for your environment:
   ```bash
   nano /etc/simple-tftpd/simple-tftpd.conf
   ```

3. **Validate the configuration**:
   ```bash
   simple-tftpd --config /etc/simple-tftpd/simple-tftpd.conf --validate
   ```

4. **Start the server**:
   ```bash
   simple-tftpd --config /etc/simple-tftpd/simple-tftpd.conf
   ```

## Configuration Validation

All configuration files should be validated before use. The server will perform basic validation on startup, but you can also validate manually:

```bash
# Validate configuration file
simple-tftpd --config your-config.conf --validate

# Check configuration syntax
simple-tftpd --config your-config.conf --check-syntax
```

## Contributing

When adding new configuration examples:

1. **Follow the naming convention**: `descriptive_name.conf.example`
2. **Include comprehensive comments** explaining each section
3. **Add to the appropriate complexity directory**
4. **Update this README** with a description
5. **Test the configuration** before committing

## Notes

- All examples use relative paths where possible
- Security settings are conservative by default
- Performance settings are balanced for general use
- Production deployments should customize based on specific requirements

---

*For more information, see the main project documentation and roadmap.*
