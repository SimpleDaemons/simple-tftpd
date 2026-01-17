# Performance Testing Guide

**Date:** January 2025  
**Purpose:** Guide for running and interpreting performance tests

---

## Overview

Performance testing ensures Simple TFTP Daemon meets production performance requirements. The test suite includes:

1. **Throughput Tests** - Measure transfer speeds
2. **Windowed Transfer Performance** - Compare window sizes
3. **Block Size Performance** - Compare block sizes
4. **Sequential Transfer Performance** - Multiple file transfers

---

## Quick Start

### Using the Automated Script

```bash
# Run all performance tests
./scripts/run-performance-tests.sh
```

This script will:
- Run all performance tests
- Generate detailed reports
- Save results with timestamps
- Create a summary report

---

## Manual Execution

### Prerequisites

```bash
# Build with tests enabled
cd build
cmake .. -DENABLE_TESTS=ON
make -j$(nproc)
```

### Running Performance Tests

```bash
# Run all performance tests
./bin/simple-tftpd-integration-tests --gtest_filter="PerformanceTestFixture.*"

# Run specific test
./bin/simple-tftpd-integration-tests --gtest_filter="PerformanceTestFixture.BasicThroughput"
./bin/simple-tftpd-integration-tests --gtest_filter="PerformanceTestFixture.WindowedTransferPerformance"
./bin/simple-tftpd-integration-tests --gtest_filter="PerformanceTestFixture.BlockSizePerformance"
./bin/simple-tftpd-integration-tests --gtest_filter="PerformanceTestFixture.SequentialTransfers"
```

---

## Test Descriptions

### 1. Basic Throughput Test

**Purpose:** Measure basic read/write throughput

**What it tests:**
- Single file transfer (1MB)
- Write performance
- Read performance
- Throughput in Mbps

**Expected results:**
- Write throughput: > 10 Mbps (local network)
- Read throughput: > 10 Mbps (local network)
- Completion time: < 10 seconds for 1MB

**Example output:**
```
Write throughput: 45.2 Mbps (234 ms)
Read throughput: 48.7 Mbps (215 ms)
```

### 2. Windowed Transfer Performance

**Purpose:** Compare performance with different window sizes

**What it tests:**
- Window size 1 (default) vs window size 4
- Transfer time comparison
- Performance improvement with windowing

**Expected results:**
- Window size 4 should be faster or equal to window size 1
- Larger windows reduce round-trips

**Example output:**
```
Window size 1: 1234 ms
Window size 4: 856 ms
Improvement: 30.6%
```

### 3. Block Size Performance

**Purpose:** Compare performance with different block sizes

**What it tests:**
- Block size 512 (default) vs 1024
- Transfer time comparison
- Performance improvement with larger blocks

**Expected results:**
- Larger blocks should be faster (fewer round-trips)
- Diminishing returns after 1024 bytes

**Example output:**
```
Block size 512: 1234 ms
Block size 1024: 987 ms
Improvement: 20.0%
```

### 4. Sequential Transfer Performance

**Purpose:** Measure performance with multiple sequential transfers

**What it tests:**
- Multiple file transfers (10 files, 64KB each)
- Average transfer time
- Total time
- Consistency across transfers

**Expected results:**
- Consistent performance across transfers
- Average transfer time should be reasonable
- No significant degradation over time

**Example output:**
```
Average transfer time for 10 files: 125 ms
Total time: 1250 ms
```

---

## Performance Benchmarks

### Target Metrics

| Metric | Target | Notes |
|--------|--------|-------|
| Throughput (local) | > 10 Mbps | Local network |
| Throughput (LAN) | > 5 Mbps | Local area network |
| Latency | < 100 ms | First packet response |
| Concurrent connections | > 100 | Simultaneous transfers |
| Memory per connection | < 50 KB | Per active connection |
| CPU usage (idle) | < 1% | No active transfers |
| CPU usage (active) | < 50% | Under load |

### Baseline Results

These are example baseline results from development environment:

```
Platform: Linux x86_64
CPU: Intel Core i7-8700K
Network: Localhost (127.0.0.1)

Basic Throughput:
  Write: 45.2 Mbps (234 ms for 1MB)
  Read: 48.7 Mbps (215 ms for 1MB)

Windowed Transfer:
  Window 1: 1234 ms
  Window 4: 856 ms (30.6% improvement)

Block Size:
  Block 512: 1234 ms
  Block 1024: 987 ms (20.0% improvement)

Sequential Transfers:
  Average: 125 ms per file (64KB)
  Total: 1250 ms for 10 files
```

---

## Interpreting Results

### Good Performance Indicators

✅ **Consistent throughput** - Similar speeds across runs  
✅ **Low latency** - Fast response times  
✅ **Scalable** - Performance doesn't degrade with load  
✅ **Resource efficient** - Low CPU and memory usage  

### Performance Issues

⚠️ **Degrading performance** - Slower over time (memory leak?)  
⚠️ **High CPU usage** - Inefficient algorithms  
⚠️ **High memory usage** - Memory leaks or inefficient data structures  
⚠️ **Inconsistent results** - Unstable performance  

---

## Load Testing

### Concurrent Connection Test

```bash
# Run multiple clients simultaneously
for i in {1..10}; do
    tftp localhost << EOF &
    get test_file_$i.bin
    quit
EOF
done
wait
```

### Stress Test

```bash
# Run many transfers in parallel
seq 1 100 | xargs -P 10 -I {} tftp localhost << EOF
get test_file_{}.bin
quit
EOF
```

---

## Performance Profiling

### Using perf (Linux)

```bash
# Profile the server
perf record -g ./bin/simple-tftpd start
perf report
```

### Using Instruments (macOS)

```bash
# Profile with Instruments
instruments -t "Time Profiler" ./bin/simple-tftpd start
```

### Using gprof

```bash
# Build with profiling
cmake .. -DCMAKE_CXX_FLAGS="-pg"
make
./bin/simple-tftpd start
gprof ./bin/simple-tftpd gmon.out > profile.txt
```

---

## Optimization Tips

1. **Use appropriate window sizes** - Larger windows for high-latency networks
2. **Use larger block sizes** - Fewer round-trips
3. **Monitor resource usage** - CPU, memory, network
4. **Profile hot paths** - Identify bottlenecks
5. **Test under load** - Real-world conditions

---

## Continuous Performance Monitoring

### CI/CD Integration

```yaml
- name: Performance Tests
  run: |
    ./scripts/run-performance-tests.sh
    # Compare with baseline
    python scripts/compare-performance.py baseline.json performance-results/
```

### Performance Regression Detection

```bash
# Compare with previous run
python scripts/compare-performance.py \
    previous_results.json \
    current_results.json \
    --threshold 0.1  # 10% regression threshold
```

---

## Troubleshooting

### Slow Performance

**Check:**
- Network latency
- Block size configuration
- Window size configuration
- System resource usage
- Other processes competing for resources

### Inconsistent Results

**Check:**
- System load
- Network conditions
- Background processes
- Thermal throttling (CPU)

---

## References

- [Performance Testing Best Practices](https://www.owasp.org/index.php/Performance_Testing)
- [TFTP Performance Optimization](https://tools.ietf.org/html/rfc2347)

---

*Last Updated: January 2025*
