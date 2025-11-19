# Configuration Examples

This directory contains example configuration files for the Simple TFTP Daemon, organized by complexity and use case. Examples are provided in **JSON**, **YAML**, and **INI** formats for reference.

## Directory Structure

```
config/examples/
├── simple/              # Basic configurations for common use cases
├── advanced/            # Advanced configurations with additional features
├── production/          # Production-ready configurations
└── README.md           # This file
```

## Configuration Formats

### JSON Configuration ✅
- **File Extension**: `.json` or `.conf`
- **Status**: ✅ Fully supported
- **Use Case**: Primary configuration format (currently implemented)

### YAML Configuration 🔄
- **File Extension**: `.yaml` or `.yml`
- **Status**: 🔄 Planned for future release
- **Use Case**: Alternative configuration format for easier readability
- **Note**: Examples provided for reference only

### INI Configuration 🔄
- **File Extension**: `.ini` or `.conf`
- **Status**: 🔄 Planned for future release
- **Use Case**: Simple key-value configuration format
- **Note**: Examples provided for reference only

## Simple Configurations

### Basic Server
Minimal configuration for basic TFTP server setup. Includes only essential settings.

**Files:**
- `basic_server.json.example` - JSON format (✅ supported)
- `basic_server.yaml.example` - YAML format (🔄 planned)
- `basic_server.ini.example` - INI format (🔄 planned)

**Best for**: Quick setup, testing, development environments

### Development
Development-friendly configuration with debugging enabled and local paths.

**Files:**
- `development.json.example` - JSON format (✅ supported)
- `development.yaml.example` - YAML format (🔄 planned)
- `development.ini.example` - INI format (🔄 planned)

**Best for**: Development, testing, local development environments

### Read-Only
Secure read-only configuration for serving files without write access.

**Files:**
- `read_only.json.example` - JSON format (✅ supported)
- `read_only.yaml.example` - YAML format (🔄 planned)
- `read_only.ini.example` - INI format (🔄 planned)

**Best for**: Public file servers, firmware distribution, ISO repositories

## Advanced Configurations

### Secure Server
Advanced security features including access control, file filtering, and network restrictions.

**Files:**
- `secure_server.json.example` - JSON format (✅ supported)
- `secure_server.yaml.example` - YAML format (🔄 planned)
- `secure_server.ini.example` - INI format (🔄 planned)

**Features:**
- Client IP allowlists
- File extension filtering
- Directory restrictions
- Enhanced logging

**Best for**: Production environments requiring high security

### High Performance
Performance-optimized configuration with large block sizes and windowed transfers.

**Files:**
- `high_performance.json.example` - JSON format (✅ supported)
- `high_performance.yaml.example` - YAML format (🔄 planned)
- `high_performance.ini.example` - INI format (🔄 planned)

**Features:**
- Large block sizes (8KB)
- Windowed transfers (window size 16)
- Optimized timeouts
- Reduced logging overhead

**Best for**: High-throughput scenarios, large file transfers

### Multi-Tenant
Configuration supporting isolated environments for different user groups.

**Files:**
- `multi_tenant.json.example` - JSON format (✅ supported)
- `multi_tenant.yaml.example` - YAML format (🔄 planned)
- `multi_tenant.ini.example` - INI format (🔄 planned)

**Best for**: Shared infrastructure, hosting providers, enterprise environments

## Production Configurations

### Enterprise
Production-ready configuration for enterprise environments with comprehensive security and performance settings.

**Files:**
- `enterprise.json.example` - JSON format (✅ supported)
- `enterprise.yaml.example` - YAML format (🔄 planned)
- `enterprise.ini.example` - INI format (🔄 planned)

**Features:**
- Network segmentation (private IP ranges)
- File extension allowlists
- Optimized block sizes
- Structured logging

**Best for**: Enterprise deployments, corporate networks

### Firmware Distribution
Optimized for firmware and boot image distribution with large file support.

**Files:**
- `firmware_distribution.json.example` - JSON format (✅ supported)
- `firmware_distribution.yaml.example` - YAML format (🔄 planned)
- `firmware_distribution.ini.example` - INI format (🔄 planned)

**Features:**
- Large file support (2GB+)
- Firmware-specific extensions (.bin, .img, .rom, .fw, .hex)
- High retry count for unreliable networks
- Large window sizes for faster transfers

**Best for**: Network device firmware updates, PXE boot servers, embedded systems

### High Availability
Configuration for HA deployments with shared storage and failover support.

**Files:**
- `high_availability.json.example` - JSON format (✅ supported)
- `high_availability.yaml.example` - YAML format (🔄 planned)
- `high_availability.ini.example` - INI format (🔄 planned)

**Features:**
- Shared storage paths
- Read/write enabled for replication
- Balanced performance settings
- HA-optimized logging

**Best for**: High availability deployments, clustered environments

## Usage

### 1. Copy the appropriate example

```bash
# For JSON (currently supported)
cp config/examples/simple/basic_server.json.example /etc/simple-tftpd/simple-tftpd.conf

# For YAML (when supported)
cp config/examples/simple/basic_server.yaml.example /etc/simple-tftpd/simple-tftpd.yaml

# For INI (when supported)
cp config/examples/simple/basic_server.ini.example /etc/simple-tftpd/simple-tftpd.ini
```

### 2. Edit the configuration

```bash
nano /etc/simple-tftpd/simple-tftpd.conf
```

### 3. Validate the configuration

```bash
simple-tftpd --config /etc/simple-tftpd/simple-tftpd.conf --validate
```

### 4. Start the server

```bash
simple-tftpd --config /etc/simple-tftpd/simple-tftpd.conf start
```

## Configuration Format Comparison

### JSON Format (Current)
```json
{
    "network": {
        "listen_address": "0.0.0.0",
        "listen_port": 69
    },
    "security": {
        "read_enabled": true
    }
}
```

### YAML Format (Planned)
```yaml
network:
  listen_address: "0.0.0.0"
  listen_port: 69

security:
  read_enabled: true
```

### INI Format (Planned)
```ini
[network]
listen_address = 0.0.0.0
listen_port = 69

[security]
read_enabled = true
```

## Configuration Validation

All configuration files should be validated before use:

```bash
# Validate configuration file
simple-tftpd --config your-config.json --validate

# Test configuration syntax
simple-tftpd --config your-config.json --test-config
```

## Hot Reload

After modifying a configuration file, reload it without restarting the server:

```bash
# Send SIGHUP signal to reload configuration
killall -HUP simple-tftpd

# Or use systemd
systemctl reload simple-tftpd
```

**Note**: Network settings (listen address, port, IPv6) require a full restart to take effect.

## Best Practices

1. **Start Simple**: Begin with `basic_server.json.example` and add features as needed
2. **Security First**: Use `read_only.json.example` for public-facing servers
3. **Test Changes**: Always validate configuration before applying
4. **Use Hot Reload**: Use SIGHUP for configuration changes when possible
5. **Monitor Logs**: Check logs after configuration changes
6. **Backup Configs**: Keep backups of working configurations

## Contributing

When adding new configuration examples:

1. **Follow naming convention**: `descriptive_name.{json|yaml|ini}.example`
2. **Include comprehensive comments** explaining each section
3. **Add to appropriate complexity directory** (simple/advanced/production)
4. **Create all three formats** (JSON, YAML, INI) for consistency
5. **Update this README** with a description
6. **Test the configuration** before committing

## Notes

- All examples use relative paths where possible
- Security settings are conservative by default
- Performance settings are balanced for general use
- Production deployments should customize based on specific requirements
- YAML and INI formats are provided for reference; JSON is currently the only supported format

---

*For more information, see the main project documentation and roadmap.*
