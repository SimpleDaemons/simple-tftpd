# Simple TFTP Daemon - Troubleshooting Guide

**Version:** v0.2.0-beta  
**Last Updated:** December 2024

---

## Quick Reference

### Common Commands

```bash
# Check server status
simple-tftpd status

# View logs
tail -f /var/log/simple-tftpd.log

# Test configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Restart service
sudo systemctl restart simple-tftpd

# Reload configuration
sudo systemctl reload simple-tftpd
```

---

## Error Codes

### TFTP Error Codes

| Code | Name | Description | Solution |
|------|------|-------------|----------|
| 0 | Not Defined | Undefined error | Check logs for details |
| 1 | File Not Found | File does not exist | Verify file path and permissions |
| 2 | Access Violation | Permission denied | Check file permissions and security settings |
| 3 | Disk Full | No space available | Free up disk space |
| 4 | Illegal Operation | Invalid TFTP operation | Check client implementation |
| 5 | Unknown Transfer ID | Connection mismatch | Usually transient, retry |
| 6 | File Already Exists | Overwrite protection | Disable overwrite protection or use different filename |
| 7 | No Such User | Authentication failed | Check user configuration |

---

## Common Problems & Solutions

### Problem: Server Won't Start

#### Symptoms
- Service fails to start
- Error messages in logs
- Port binding errors

#### Diagnosis

```bash
# Check if port is in use
sudo netstat -ulnp | grep :69
sudo lsof -i :69

# Check configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Check logs
sudo journalctl -u simple-tftpd -n 50
tail -50 /var/log/simple-tftpd.log
```

#### Solutions

1. **Port Already in Use**
   ```bash
   # Find process using port 69
   sudo fuser 69/udp
   
   # Kill process (if safe)
   sudo killall <process-name>
   
   # Or use different port
   # Edit config: "listen_port": 6969
   ```

2. **Permission Denied**
   ```bash
   # Check if running as correct user
   ps aux | grep simple-tftpd
   
   # Fix permissions
   sudo chown -R tftp:tftp /var/tftp
   sudo chown tftp:tftp /var/log/simple-tftpd.log
   ```

3. **Invalid Configuration**
   ```bash
   # Validate configuration
   simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf
   
   # Check JSON syntax
   cat /etc/simple-tftpd/simple-tftpd.conf | jq .
   ```

---

### Problem: Clients Cannot Connect

#### Symptoms
- Connection refused errors
- Timeout errors
- No response from server

#### Diagnosis

```bash
# Check if server is running
sudo systemctl status simple-tftpd

# Check if listening on correct interface
sudo netstat -ulnp | grep simple-tftpd

# Test from server itself
tftp localhost 69
tftp> get testfile.txt

# Check firewall
sudo iptables -L -n | grep 69
sudo firewall-cmd --list-all
```

#### Solutions

1. **Firewall Blocking**
   ```bash
   # Allow TFTP (iptables)
   sudo iptables -A INPUT -p udp --dport 69 -j ACCEPT
   
   # Allow TFTP (firewalld)
   sudo firewall-cmd --permanent --add-service=tftp
   sudo firewall-cmd --reload
   
   # Allow TFTP (ufw)
   sudo ufw allow 69/udp
   ```

2. **Wrong Listen Address**
   ```json
   {
       "network": {
           "listen_address": "0.0.0.0"  // Listen on all interfaces
       }
   }
   ```

3. **Client Filtering**
   ```json
   {
       "security": {
           "allowed_clients": []  // Empty = allow all
       }
   }
   ```

---

### Problem: File Transfers Fail

#### Symptoms
- "File Not Found" errors
- "Access Violation" errors
- Transfers start but fail

#### Diagnosis

```bash
# Check file exists
ls -la /var/tftp/public/testfile.txt

# Check permissions
ls -la /var/tftp/

# Check configuration
simple-tftpd status --json | jq '.filesystem'

# Test file access
sudo -u tftp cat /var/tftp/public/testfile.txt
```

#### Solutions

1. **File Not Found**
   ```bash
   # Verify file exists
   ls -la /var/tftp/public/
   
   # Check root directory
   cat /etc/simple-tftpd/simple-tftpd.conf | jq '.filesystem.root_directory'
   
   # Verify allowed directories
   cat /etc/simple-tftpd/simple-tftpd.conf | jq '.filesystem.allowed_directories'
   ```

2. **Permission Denied**
   ```bash
   # Fix file permissions
   sudo chmod 644 /var/tftp/public/*
   sudo chown tftp:tftp /var/tftp/public/*
   
   # Fix directory permissions
   sudo chmod 755 /var/tftp/public
   sudo chown tftp:tftp /var/tftp/public
   ```

3. **Extension Filtering**
   ```json
   {
       "security": {
           "allowed_extensions": ["bin", "img", "txt"]  // Add your extension
       }
   }
   ```

4. **File Size Limit**
   ```json
   {
       "security": {
           "max_file_size": 104857600  // Increase if needed
       }
   }
   ```

---

### Problem: Slow Transfer Speeds

#### Symptoms
- Transfers are slower than expected
- High latency
- Frequent timeouts

#### Diagnosis

```bash
# Check network speed
iperf3 -c <server-ip> -u

# Check server load
top -p $(pgrep simple-tftpd)

# Check disk I/O
iostat -x 1

# Check logs for errors
grep -i "timeout\|retry" /var/log/simple-tftpd.log
```

#### Solutions

1. **Increase Block Size**
   ```json
   {
       "performance": {
           "block_size": 1024  // or 1400 for MTU-optimized
       }
   }
   ```

2. **Enable Windowed Transfers**
   ```json
   {
       "performance": {
           "window_size": 4  // Increase for reliable networks
       }
   }
   ```

3. **Adjust Timeout**
   ```json
   {
       "performance": {
           "timeout": 10,  // Increase for high-latency networks
           "max_retries": 5
       }
   }
   ```

4. **Disk I/O Issues**
   ```bash
   # Use faster storage (SSD)
   # Move root directory to faster disk
   # Check disk health
   sudo smartctl -a /dev/sda
   ```

---

### Problem: High Memory Usage

#### Symptoms
- Server using excessive memory
- System running out of memory
- OOM killer terminating process

#### Diagnosis

```bash
# Check memory usage
ps aux | grep simple-tftpd
top -p $(pgrep simple-tftpd)

# Check for memory leaks
valgrind --leak-check=full simple-tftpd start

# Monitor over time
watch -n 1 'ps aux | grep simple-tftpd'
```

#### Solutions

1. **Too Many Connections**
   ```bash
   # Check active connections
   simple-tftpd connections
   
   # Limit connections (if implemented)
   # Add connection limit to config
   ```

2. **Memory Leak**
   ```bash
   # Run with AddressSanitizer
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZER=ON
   make
   
   # Run tests with leak detection
   valgrind --leak-check=full ./bin/simple-tftpd-tests
   ```

3. **Large File Buffers**
   ```json
   {
       "performance": {
           "block_size": 512  // Reduce if memory constrained
       }
   }
   ```

---

### Problem: Configuration Not Reloading

#### Symptoms
- Changes to config file not taking effect
- Reload command fails
- Server still using old settings

#### Diagnosis

```bash
# Check if reload signal is sent
sudo killall -HUP simple-tftpd

# Check logs for reload messages
grep -i "reload\|config" /var/log/simple-tftpd.log

# Verify config file is readable
sudo -u tftp cat /etc/simple-tftpd/simple-tftpd.conf
```

#### Solutions

1. **Invalid Configuration**
   ```bash
   # Validate before reloading
   simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf
   
   # Check JSON syntax
   cat /etc/simple-tftpd/simple-tftpd.conf | jq .
   ```

2. **Permission Issues**
   ```bash
   # Ensure config file is readable
   sudo chmod 644 /etc/simple-tftpd/simple-tftpd.conf
   sudo chown root:root /etc/simple-tftpd/simple-tftpd.conf
   ```

3. **Restart Instead of Reload**
   ```bash
   # If reload doesn't work, restart
   sudo systemctl restart simple-tftpd
   ```

---

### Problem: Logs Not Appearing

#### Symptoms
- No log file created
- Empty log file
- Logs not written

#### Diagnosis

```bash
# Check log file exists
ls -la /var/log/simple-tftpd.log

# Check permissions
ls -la /var/log/simple-tftpd.log

# Check configuration
cat /etc/simple-tftpd/simple-tftpd.conf | jq '.logging'

# Test logging
simple-tftpd start --verbose
```

#### Solutions

1. **Permission Denied**
   ```bash
   # Create log file with correct permissions
   sudo touch /var/log/simple-tftpd.log
   sudo chown tftp:tftp /var/log/simple-tftpd.log
   sudo chmod 644 /var/log/simple-tftpd.log
   ```

2. **Log Directory Missing**
   ```bash
   # Create log directory
   sudo mkdir -p /var/log
   sudo chown tftp:tftp /var/log
   ```

3. **Logging Disabled**
   ```json
   {
       "logging": {
           "level": "INFO",  // Enable logging
           "log_file": "/var/log/simple-tftpd.log"
       }
   }
   ```

---

## Debug Procedures

### Enable Debug Logging

```json
{
    "logging": {
        "level": "DEBUG",
        "console_logging": true
    }
}
```

Or via command line:

```bash
simple-tftpd start --verbose --config /etc/simple-tftpd/simple-tftpd.conf
```

### Test Configuration

```bash
# Validate configuration
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf

# Test with verbose output
simple-tftpd test --config /etc/simple-tftpd/simple-tftpd.conf --verbose
```

### Network Diagnostics

```bash
# Test UDP connectivity
nc -u -v localhost 69

# Test from remote
tftp <server-ip> 69
tftp> get testfile.txt

# Capture packets
sudo tcpdump -i any -n udp port 69
```

### Performance Profiling

```bash
# Profile with perf (Linux)
sudo perf record -p $(pgrep simple-tftpd)
sudo perf report

# Check system resources
htop
iotop
```

---

## Error Message Reference

### Server Errors

| Error | Cause | Solution |
|-------|-------|----------|
| "Failed to bind socket" | Port in use or permission denied | Check port availability, run as root or use capabilities |
| "Failed to initialize socket" | Network subsystem issue | Check network configuration, restart network service |
| "Configuration file not found" | Config file missing | Create config file or specify correct path |
| "Invalid configuration" | JSON syntax error | Validate JSON, check configuration file |
| "Permission denied" | File system permissions | Fix file/directory permissions |

### Transfer Errors

| Error | Cause | Solution |
|-------|-------|----------|
| "File not found" | File doesn't exist | Verify file path, check root directory |
| "Access violation" | Permission denied | Check file permissions, security settings |
| "Disk full" | No space available | Free up disk space |
| "File already exists" | Overwrite protection | Disable protection or use different filename |
| "Transfer timeout" | Network issues | Increase timeout, check network stability |

---

## Getting Help

### Information to Provide

When reporting issues, include:

1. **Version Information**
   ```bash
   simple-tftpd --version
   ```

2. **Configuration**
   ```bash
   cat /etc/simple-tftpd/simple-tftpd.conf
   ```

3. **Logs**
   ```bash
   tail -100 /var/log/simple-tftpd.log
   ```

4. **System Information**
   ```bash
   uname -a
   cat /etc/os-release
   ```

5. **Status**
   ```bash
   simple-tftpd status --json
   ```

### Support Channels

- **GitHub Issues:** https://github.com/simpledaemons/simple-tftpd/issues
- **Documentation:** See `docs/` directory
- **Logs:** Check `/var/log/simple-tftpd.log`

---

*Last Updated: December 2024*
