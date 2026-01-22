# Debugging Guide

This guide covers debugging techniques for Simple TFTP Daemon.

## Overview

Debugging helps identify and resolve issues with Simple TFTP Daemon. This guide provides techniques and tools for effective debugging.

## Debug Logging

### Enable Debug Logging

```json
{
  "logging": {
    "level": "DEBUG",
    "log_file": "/var/log/simple-tftpd/simple-tftpd.log",
    "console_logging": true,
    "debug_categories": [
      "packet",
      "connection",
      "file",
      "network"
    ]
  }
}
```

### Verbose Mode

```bash
# Run with verbose output
simple-tftpd start --config /etc/simple-tftpd/simple-tftpd.conf --verbose

# Or set environment variable
export TFTP_DEBUG=1
simple-tftpd start
```

## Log Analysis

### View Logs

```bash
# Recent logs
tail -f /var/log/simple-tftpd/simple-tftpd.log

# Error logs only
grep ERROR /var/log/simple-tftpd/simple-tftpd.log

# Specific time range
journalctl -u simple-tftpd --since "1 hour ago"

# Follow logs
journalctl -u simple-tftpd -f
```

### Log Patterns

**Connection Issues:**
```bash
grep -i "connection" /var/log/simple-tftpd/simple-tftpd.log
```

**Packet Issues:**
```bash
grep -i "packet" /var/log/simple-tftpd/simple-tftpd.log
```

**File Issues:**
```bash
grep -i "file" /var/log/simple-tftpd/simple-tftpd.log
```

## Network Debugging

### Packet Capture

```bash
# Capture TFTP packets
sudo tcpdump -i any -n port 69 -w tftp.pcap

# Analyze with Wireshark
wireshark tftp.pcap
```

### Network Connectivity

```bash
# Test UDP connectivity
nc -u -v localhost 69

# Check port binding
sudo netstat -tulpn | grep 69

# Check firewall
sudo firewall-cmd --list-all
```

### Connection Testing

```bash
# Test TFTP connection
tftp localhost
tftp> get test.txt
tftp> quit

# Verbose TFTP client
tftp -v localhost
```

## Configuration Debugging

### Validate Configuration

```bash
# Test configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Check JSON syntax
python -m json.tool /etc/simple-tftpd/simple-tftpd.conf

# Validate with schema
simple-tftpd validate --config /etc/simple-tftpd/simple-tftpd.conf
```

### Configuration Issues

**Common Issues:**
- Invalid JSON syntax
- Missing required fields
- Invalid values
- Permission issues

**Debug Steps:**
1. Validate JSON syntax
2. Check required fields
3. Verify values
4. Check file permissions

## File System Debugging

### File Access Issues

```bash
# Check file permissions
ls -la /var/tftp

# Check directory permissions
ls -ld /var/tftp

# Test file access
sudo -u tftp cat /var/tftp/test.txt
```

### Path Issues

```bash
# Check path traversal
grep -i "path" /var/log/simple-tftpd/simple-tftpd.log

# Verify allowed directories
grep allowed_directories /etc/simple-tftpd/simple-tftpd.conf
```

## Performance Debugging

### Resource Usage

```bash
# CPU usage
top -p $(pgrep simple-tftpd)

# Memory usage
ps aux | grep simple-tftpd

# File descriptors
lsof -p $(pgrep simple-tftpd) | wc -l
```

### Performance Profiling

```bash
# CPU profiling
perf record -g -p $(pgrep simple-tftpd)
perf report

# Memory profiling
valgrind --tool=massif simple-tftpd
```

## Code Debugging

### GDB Debugging

```bash
# Attach to running process
sudo gdb -p $(pgrep simple-tftpd)

# Set breakpoints
(gdb) break server.cpp:100

# Run with debugger
gdb ./simple-tftpd
(gdb) run start --config /etc/simple-tftpd/simple-tftpd.conf
```

### Core Dumps

```bash
# Enable core dumps
ulimit -c unlimited

# Analyze core dump
gdb ./simple-tftpd core
```

## Common Debugging Scenarios

### Server Won't Start

**Debug Steps:**
1. Check logs
2. Validate configuration
3. Check port availability
4. Check permissions
5. Check dependencies

```bash
# Check logs
sudo journalctl -u simple-tftpd -n 50

# Test configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Check port
sudo netstat -tulpn | grep 69
```

### Connection Failures

**Debug Steps:**
1. Check network connectivity
2. Check firewall rules
3. Check client filtering
4. Check logs
5. Capture packets

```bash
# Test connectivity
nc -u -v localhost 69

# Check firewall
sudo firewall-cmd --list-all

# Check client filtering
grep allowed_clients /etc/simple-tftpd/simple-tftpd.conf
```

### File Transfer Failures

**Debug Steps:**
1. Check file permissions
2. Check file existence
3. Check directory access
4. Check file size limits
5. Check logs

```bash
# Check file
ls -la /var/tftp/test.txt

# Check permissions
stat /var/tftp/test.txt

# Check logs
grep -i "file" /var/log/simple-tftpd/simple-tftpd.log
```

## Debugging Tools

### Built-in Tools

- `simple-tftpd test` - Test configuration
- `simple-tftpd health` - Health check
- `simple-tftpd metrics` - Performance metrics
- `simple-tftpd status` - Server status

### External Tools

- **tcpdump**: Packet capture
- **Wireshark**: Packet analysis
- **strace**: System call tracing
- **ltrace**: Library call tracing
- **gdb**: Debugger
- **valgrind**: Memory debugging

## Debugging Best Practices

1. **Enable Debug Logging**: Use DEBUG log level
2. **Capture Evidence**: Save logs and packet captures
3. **Reproduce**: Reproduce issues consistently
4. **Isolate**: Isolate the problem
5. **Document**: Document findings
6. **Test Fixes**: Test fixes thoroughly

## Debugging Checklist

- [ ] Debug logging enabled
- [ ] Logs reviewed
- [ ] Configuration validated
- [ ] Network connectivity tested
- [ ] File permissions checked
- [ ] Resource usage monitored
- [ ] Packets captured (if needed)
- [ ] Issue reproduced
- [ ] Root cause identified
- [ ] Fix tested

## Next Steps

- **[Troubleshooting Guide](README.md)** - Common issues
- **[Performance Troubleshooting](performance.md)** - Performance issues
- **[Configuration Reference](../../configuration/README.md)** - Configuration help

---

**Last Updated**: December 2024
