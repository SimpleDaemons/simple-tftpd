# Performance Troubleshooting

This guide covers troubleshooting performance issues with Simple TFTP Daemon.

## Overview

Performance issues can manifest as slow transfers, high resource usage, or connection failures. This guide helps identify and resolve performance problems.

## Identifying Performance Issues

### Symptoms

- Slow file transfers
- High CPU usage
- High memory usage
- Connection timeouts
- Dropped connections
- Low throughput

### Metrics to Monitor

- Transfer rate (bytes/second)
- Response time (latency)
- CPU usage
- Memory usage
- Connection count
- Error rate

## Common Performance Issues

### Slow Transfers

**Causes:**
- Network bandwidth limitations
- Small block size
- High latency
- Disk I/O bottlenecks
- CPU limitations

**Solutions:**

```json
{
  "performance": {
    "block_size": 8192,  // Increase block size
    "window_size": 16,   // Increase window size
    "timeout": 10        // Increase timeout
  }
}
```

**Network Optimization:**
```bash
# Check network speed
iperf3 -c server-ip

# Check latency
ping server-ip

# Check bandwidth
iftop -i eth0
```

### High CPU Usage

**Causes:**
- Too many connections
- Inefficient processing
- High packet rate
- Compression overhead

**Solutions:**

```json
{
  "performance": {
    "max_connections": 1000,  // Limit connections
    "thread_pool_size": 8     // Adjust thread pool
  }
}
```

**System Optimization:**
```bash
# Check CPU usage
top -p $(pgrep simple-tftpd)

# Check process threads
ps -T -p $(pgrep simple-tftpd)

# Profile CPU
perf top -p $(pgrep simple-tftpd)
```

### High Memory Usage

**Causes:**
- Too many connections
- Large buffers
- Memory leaks
- Cache size

**Solutions:**

```json
{
  "performance": {
    "max_connections": 1000,
    "buffer_size": 65536,
    "cache_size": "1Gi"
  }
}
```

**Memory Analysis:**
```bash
# Check memory usage
ps aux | grep simple-tftpd

# Check memory leaks
valgrind --leak-check=full simple-tftpd

# Monitor memory
watch -n 1 'ps aux | grep simple-tftpd'
```

### Connection Timeouts

**Causes:**
- Network latency
- Slow disk I/O
- High load
- Timeout too short

**Solutions:**

```json
{
  "performance": {
    "timeout": 30,        // Increase timeout
    "max_retries": 5      // Increase retries
  }
}
```

**Network Debugging:**
```bash
# Check latency
ping server-ip

# Check packet loss
mtr server-ip

# Check network quality
iperf3 -c server-ip -u
```

### Low Throughput

**Causes:**
- Network bandwidth limits
- Small window size
- Disk I/O bottlenecks
- CPU limitations

**Solutions:**

```json
{
  "performance": {
    "window_size": 32,     // Increase window size
    "block_size": 8192,   // Increase block size
    "async_io": true      // Enable async I/O
  }
}
```

**Throughput Testing:**
```bash
# Test throughput
iperf3 -c server-ip

# Monitor network
iftop -i eth0

# Check disk I/O
iostat -x 1
```

## System-Level Optimization

### Kernel Parameters

```bash
# Network parameters
sudo sysctl -w net.core.somaxconn=65535
sudo sysctl -w net.ipv4.tcp_max_syn_backlog=65535
sudo sysctl -w net.ipv4.ip_local_port_range="10000 65535"

# File system parameters
sudo sysctl -w fs.file-max=2097152
sudo sysctl -w vm.swappiness=10
```

### Resource Limits

```bash
# Increase file descriptors
ulimit -n 65536

# Check current limits
ulimit -a
```

### CPU Affinity

```bash
# Set CPU affinity
taskset -cp 0-3 $(pgrep simple-tftpd)
```

## Network Optimization

### Network Tuning

```bash
# Increase receive buffer
sudo sysctl -w net.core.rmem_max=16777216
sudo sysctl -w net.core.rmem_default=16777216

# Increase send buffer
sudo sysctl -w net.core.wmem_max=16777216
sudo sysctl -w net.core.wmem_default=16777216
```

### Network Interface

```bash
# Check interface speed
ethtool eth0

# Enable offloading
sudo ethtool -K eth0 gro on
sudo ethtool -K eth0 tso on
```

## Disk I/O Optimization

### Disk Performance

```bash
# Check disk I/O
iostat -x 1

# Check disk usage
df -h

# Test disk speed
dd if=/dev/zero of=/var/tftp/test bs=1M count=1000
```

### File System Optimization

```bash
# Use faster file system (XFS, ext4)
# Enable noatime
mount -o noatime /dev/sda1 /var/tftp

# Tune file system
tune2fs -o journal_data_writeback /dev/sda1
```

## Configuration Optimization

### Performance Settings

```json
{
  "performance": {
    "block_size": 8192,
    "window_size": 32,
    "timeout": 30,
    "max_connections": 5000,
    "thread_pool_size": 16,
    "async_io": true,
    "read_ahead": 1048576
  }
}
```

### Caching

```json
{
  "cache": {
    "enabled": true,
    "size": "4Gi",
    "ttl": 3600
  }
}
```

## Monitoring Performance

### Metrics Collection

```bash
# Check metrics
simple-tftpd metrics

# Monitor in real-time
watch -n 1 'simple-tftpd metrics'

# Export metrics
simple-tftpd metrics --format json > metrics.json
```

### Performance Profiling

```bash
# CPU profiling
perf record -g -p $(pgrep simple-tftpd)
perf report

# Memory profiling
valgrind --tool=massif simple-tftpd
```

## Troubleshooting Checklist

- [ ] Network bandwidth checked
- [ ] Network latency measured
- [ ] CPU usage monitored
- [ ] Memory usage monitored
- [ ] Disk I/O checked
- [ ] Configuration optimized
- [ ] System limits increased
- [ ] Kernel parameters tuned
- [ ] Performance metrics collected
- [ ] Bottlenecks identified

## Performance Best Practices

1. **Monitor First**: Monitor before optimizing
2. **Baseline**: Establish performance baseline
3. **Identify Bottlenecks**: Find the limiting factor
4. **Optimize Incrementally**: Make one change at a time
5. **Measure Impact**: Measure after each change
6. **Document**: Document optimizations
7. **Test**: Test in non-production first

## Next Steps

- **[Debugging Guide](debugging.md)** - Debugging techniques
- **[Troubleshooting Guide](README.md)** - Common issues
- **[Performance Tuning](../../production/performance.md)** - Performance optimization

---

**Last Updated**: December 2024
