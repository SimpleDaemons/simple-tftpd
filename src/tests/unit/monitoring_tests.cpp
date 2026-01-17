/*
 * Copyright 2024 SimpleDaemons
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "simple-tftpd/core/monitoring.hpp"
#include <thread>
#include <chrono>

using namespace simple_tftpd;

// Test fixture for monitoring tests
class MonitoringTest : public ::testing::Test {
protected:
    void SetUp() override {
        monitoring = std::make_unique<Monitoring>();
    }

    void TearDown() override {
        monitoring.reset();
    }

    std::unique_ptr<Monitoring> monitoring;
};

// Test health check
TEST_F(MonitoringTest, HealthCheck) {
    auto health = monitoring->performHealthCheck();
    
    // Initially should be healthy
    EXPECT_EQ(health.status, HealthStatus::HEALTHY);
    EXPECT_FALSE(health.message.empty());
    
    // Should have some details
    EXPECT_FALSE(health.details.empty());
}

// Test metrics collection
TEST_F(MonitoringTest, MetricsCollection) {
    auto metrics = monitoring->getMetrics();
    
    // Initial metrics should be zero or minimal
    EXPECT_EQ(metrics.transfers.total_transfers, 0);
    EXPECT_EQ(metrics.connections.total_connections, 0);
    EXPECT_EQ(metrics.total_errors, 0);
    EXPECT_EQ(metrics.total_timeouts, 0);
}

// Test transfer recording
TEST_F(MonitoringTest, TransferRecording) {
    // Record some transfers
    monitoring->recordTransfer(1024, true, 100);
    monitoring->recordTransfer(2048, true, 200);
    monitoring->recordTransfer(512, false, 50);
    
    auto stats = monitoring->getTransferStats();
    
    EXPECT_EQ(stats.total_transfers, 3);
    EXPECT_EQ(stats.successful_transfers, 2);
    EXPECT_EQ(stats.failed_transfers, 1);
    EXPECT_EQ(stats.total_bytes_sent, 3072); // 1024 + 2048
    EXPECT_GT(stats.average_transfer_time_ms, 0);
}

// Test connection recording
TEST_F(MonitoringTest, ConnectionRecording) {
    // Record some connections
    monitoring->recordConnection(true);
    monitoring->recordConnection(true);
    monitoring->recordConnection(false);
    
    auto stats = monitoring->getConnectionStats();
    
    EXPECT_EQ(stats.total_connections, 3);
    EXPECT_EQ(stats.failed_connections, 1);
}

// Test error recording
TEST_F(MonitoringTest, ErrorRecording) {
    // Record some errors
    monitoring->recordError();
    monitoring->recordError();
    monitoring->recordError();
    
    auto metrics = monitoring->getMetrics();
    EXPECT_EQ(metrics.total_errors, 3);
}

// Test timeout recording
TEST_F(MonitoringTest, TimeoutRecording) {
    // Record some timeouts
    monitoring->recordTimeout();
    monitoring->recordTimeout();
    
    auto metrics = monitoring->getMetrics();
    EXPECT_EQ(metrics.total_timeouts, 2);
}

// Test active connection updates
TEST_F(MonitoringTest, ActiveConnectionUpdates) {
    monitoring->updateActiveConnections(5);
    auto stats = monitoring->getConnectionStats();
    EXPECT_EQ(stats.active_connections, 5);
    EXPECT_EQ(stats.peak_connections, 5);
    
    monitoring->updateActiveConnections(3);
    stats = monitoring->getConnectionStats();
    EXPECT_EQ(stats.active_connections, 3);
    EXPECT_EQ(stats.peak_connections, 5); // Peak should remain
    
    monitoring->updateActiveConnections(10);
    stats = monitoring->getConnectionStats();
    EXPECT_EQ(stats.active_connections, 10);
    EXPECT_EQ(stats.peak_connections, 10); // New peak
}

// Test metrics JSON export
TEST_F(MonitoringTest, MetricsJsonExport) {
    // Record some data
    monitoring->recordTransfer(1024, true, 100);
    monitoring->recordConnection(true);
    monitoring->recordError();
    
    std::string json = monitoring->getMetricsJson();
    
    // JSON should not be empty
    EXPECT_FALSE(json.empty());
    
    // Should contain expected keys
    EXPECT_TRUE(json.find("transfers") != std::string::npos);
    EXPECT_TRUE(json.find("connections") != std::string::npos);
    EXPECT_TRUE(json.find("errors") != std::string::npos);
}

// Test health check JSON export
TEST_F(MonitoringTest, HealthCheckJsonExport) {
    auto json = monitoring->getHealthCheckJson();
    
    // JSON should not be empty
    EXPECT_FALSE(json.empty());
    
    // Should contain status and message
    EXPECT_TRUE(json.find("status") != std::string::npos);
    EXPECT_TRUE(json.find("message") != std::string::npos);
    EXPECT_TRUE(json.find("details") != std::string::npos);
}

// Test metrics reset
TEST_F(MonitoringTest, MetricsReset) {
    // Record some data
    monitoring->recordTransfer(1024, true, 100);
    monitoring->recordConnection(true);
    monitoring->recordError();
    
    // Reset metrics
    monitoring->resetMetrics();
    
    auto metrics = monitoring->getMetrics();
    EXPECT_EQ(metrics.transfers.total_transfers, 0);
    EXPECT_EQ(metrics.connections.total_connections, 0);
    EXPECT_EQ(metrics.total_errors, 0);
}

// Test health check with high error rate
TEST_F(MonitoringTest, HealthCheckHighErrorRate) {
    // Record many failed transfers
    for (int i = 0; i < 10; i++) {
        monitoring->recordTransfer(1024, false, 100);
    }
    // Record one successful transfer
    monitoring->recordTransfer(1024, true, 100);
    
    auto health = monitoring->performHealthCheck();
    
    // Should detect high error rate
    // Error rate is 10/11 = ~91%, which is > 50%
    EXPECT_TRUE(health.status == HealthStatus::DEGRADED || 
                health.status == HealthStatus::UNHEALTHY);
}

// Test health check with high connection failure rate
TEST_F(MonitoringTest, HealthCheckHighConnectionFailureRate) {
    // Record many failed connections
    for (int i = 0; i < 10; i++) {
        monitoring->recordConnection(false);
    }
    // Record a few successful connections
    for (int i = 0; i < 5; i++) {
        monitoring->recordConnection(true);
    }
    
    auto health = monitoring->performHealthCheck();
    
    // Should detect high failure rate
    // Failure rate is 10/15 = ~67%, which is > 30%
    EXPECT_TRUE(health.status == HealthStatus::DEGRADED || 
                health.status == HealthStatus::UNHEALTHY);
}

// Test concurrent access
TEST_F(MonitoringTest, ConcurrentAccess) {
    // Simulate concurrent access
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([this, i]() {
            for (int j = 0; j < 100; j++) {
                monitoring->recordTransfer(1024, true, 100);
                monitoring->recordConnection(true);
                if (j % 10 == 0) {
                    monitoring->recordError();
                }
            }
        });
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Check final metrics
    auto metrics = monitoring->getMetrics();
    EXPECT_EQ(metrics.transfers.total_transfers, 1000); // 10 threads * 100 transfers
    EXPECT_EQ(metrics.connections.total_connections, 1000);
    EXPECT_EQ(metrics.total_errors, 100); // 10 threads * 10 errors
}

// Test uptime tracking
TEST_F(MonitoringTest, UptimeTracking) {
    auto metrics1 = monitoring->getMetrics();
    auto uptime1 = metrics1.uptime.count();
    
    // Wait a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    auto metrics2 = monitoring->getMetrics();
    auto uptime2 = metrics2.uptime.count();
    
    // Uptime should have increased
    EXPECT_GE(uptime2, uptime1);
}
