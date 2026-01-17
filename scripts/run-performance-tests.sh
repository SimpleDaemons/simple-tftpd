#!/bin/bash
# Performance Test Runner
# Copyright 2024 SimpleDaemons
# Licensed under Apache License 2.0

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
RESULTS_DIR="${PROJECT_ROOT}/performance-results"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

echo "=========================================="
echo "Performance Test Runner"
echo "=========================================="
echo ""

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Please run 'make build' first."
    exit 1
fi

# Create results directory
mkdir -p "$RESULTS_DIR"

cd "$BUILD_DIR"

echo "Running performance tests..."
echo "Results will be saved to: $RESULTS_DIR"
echo ""

# Run performance tests
echo "=========================================="
echo "1. Basic Throughput Tests"
echo "=========================================="
echo ""

if ./bin/simple-tftpd-integration-tests --gtest_filter="PerformanceTestFixture.*" 2>&1 | tee "$RESULTS_DIR/throughput_${TIMESTAMP}.log"; then
    echo "✅ Throughput tests completed"
else
    echo "⚠️  Some throughput tests may have failed"
fi

echo ""
echo "=========================================="
echo "2. Windowed Transfer Performance"
echo "=========================================="
echo ""

if ./bin/simple-tftpd-integration-tests --gtest_filter="*WindowedTransferPerformance*" 2>&1 | tee "$RESULTS_DIR/windowed_${TIMESTAMP}.log"; then
    echo "✅ Windowed transfer performance tests completed"
else
    echo "⚠️  Some windowed transfer tests may have failed"
fi

echo ""
echo "=========================================="
echo "3. Block Size Performance"
echo "=========================================="
echo ""

if ./bin/simple-tftpd-integration-tests --gtest_filter="*BlockSizePerformance*" 2>&1 | tee "$RESULTS_DIR/blksize_${TIMESTAMP}.log"; then
    echo "✅ Block size performance tests completed"
else
    echo "⚠️  Some block size tests may have failed"
fi

echo ""
echo "=========================================="
echo "4. Sequential Transfer Performance"
echo "=========================================="
echo ""

if ./bin/simple-tftpd-integration-tests --gtest_filter="*SequentialTransfers*" 2>&1 | tee "$RESULTS_DIR/sequential_${TIMESTAMP}.log"; then
    echo "✅ Sequential transfer tests completed"
else
    echo "⚠️  Some sequential transfer tests may have failed"
fi

echo ""
echo "=========================================="
echo "5. Generating Performance Report"
echo "=========================================="
echo ""

# Generate summary report
REPORT_FILE="$RESULTS_DIR/performance_report_${TIMESTAMP}.md"
cat > "$REPORT_FILE" << EOF
# Performance Test Report

**Date:** $(date)
**Platform:** $(uname -s) $(uname -r)
**Build Type:** $(grep CMAKE_BUILD_TYPE "$BUILD_DIR/CMakeCache.txt" | cut -d= -f2 || echo "Unknown")

## Test Results

### Throughput Tests
\`\`\`
$(grep -A 20 "Write throughput\|Read throughput" "$RESULTS_DIR/throughput_${TIMESTAMP}.log" 2>/dev/null || echo "No throughput data found")
\`\`\`

### Windowed Transfer Performance
\`\`\`
$(grep -A 10 "Window size" "$RESULTS_DIR/windowed_${TIMESTAMP}.log" 2>/dev/null || echo "No windowed transfer data found")
\`\`\`

### Block Size Performance
\`\`\`
$(grep -A 10 "Block size" "$RESULTS_DIR/blksize_${TIMESTAMP}.log" 2>/dev/null || echo "No block size data found")
\`\`\`

### Sequential Transfer Performance
\`\`\`
$(grep -A 10 "Average transfer time\|Total time" "$RESULTS_DIR/sequential_${TIMESTAMP}.log" 2>/dev/null || echo "No sequential transfer data found")
\`\`\`

## Detailed Logs

- Throughput: \`throughput_${TIMESTAMP}.log\`
- Windowed: \`windowed_${TIMESTAMP}.log\`
- Block Size: \`blksize_${TIMESTAMP}.log\`
- Sequential: \`sequential_${TIMESTAMP}.log\`

## Next Steps

1. Review performance metrics
2. Compare with previous runs
3. Identify optimization opportunities
4. Document performance characteristics
EOF

echo "✅ Performance report generated: $REPORT_FILE"
echo ""
echo "=========================================="
echo "Summary"
echo "=========================================="
echo ""
echo "Performance tests completed!"
echo ""
echo "Results directory: $RESULTS_DIR"
echo "Report: $REPORT_FILE"
echo ""
echo "Next steps:"
echo "  1. Review the performance report"
echo "  2. Compare results with previous runs"
echo "  3. Document performance characteristics"
echo "  4. Identify optimization opportunities"
echo ""
