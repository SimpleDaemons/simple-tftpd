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

#pragma once

#include <string>
#include <chrono>
#include <map>
#include <vector>
#include <cstdint>
#include <mutex>

namespace simple_tftpd {

/**
 * @brief Health check status
 */
enum class HealthStatus {
    HEALTHY,
    DEGRADED,
    UNHEALTHY
};

/**
 * @brief Health check result
 */
struct HealthCheckResult {
    HealthStatus status;
    std::string message;
    std::map<std::string, std::string> details;
    
    HealthCheckResult() : status(HealthStatus::HEALTHY), message("OK") {}
};

/**
 * @brief Transfer statistics
 */
struct TransferStats {
    uint64_t total_transfers;
    uint64_t successful_transfers;
    uint64_t failed_transfers;
    uint64_t total_bytes_sent;
    uint64_t total_bytes_received;
    uint64_t average_transfer_time_ms;
    std::chrono::steady_clock::time_point last_transfer_time;
    
    TransferStats() : total_transfers(0), successful_transfers(0),
                     failed_transfers(0), total_bytes_sent(0),
                     total_bytes_received(0), average_transfer_time_ms(0) {}
};

/**
 * @brief Connection statistics
 */
struct ConnectionStats {
    uint64_t total_connections;
    uint64_t active_connections;
    uint64_t peak_connections;
    uint64_t failed_connections;
    std::chrono::steady_clock::time_point last_connection_time;
    
    ConnectionStats() : total_connections(0), active_connections(0),
                       peak_connections(0), failed_connections(0) {}
};

/**
 * @brief Server metrics
 */
struct ServerMetrics {
    TransferStats transfers;
    ConnectionStats connections;
    uint64_t total_errors;
    uint64_t total_timeouts;
    std::chrono::steady_clock::time_point server_start_time;
    std::chrono::seconds uptime;
    double cpu_usage_percent;
    uint64_t memory_usage_bytes;
    
    ServerMetrics() : total_errors(0), total_timeouts(0),
                     uptime(0), cpu_usage_percent(0.0),
                     memory_usage_bytes(0) {}
};

/**
 * @brief Production monitoring interface
 * 
 * Provides health checks, metrics collection, and status reporting
 * for production deployment monitoring.
 */
class Monitoring {
public:
    /**
     * @brief Constructor
     */
    Monitoring();
    
    /**
     * @brief Destructor
     */
    ~Monitoring();
    
    /**
     * @brief Perform health check
     * @return Health check result
     */
    HealthCheckResult performHealthCheck() const;
    
    /**
     * @brief Get server metrics
     * @return Server metrics
     */
    ServerMetrics getMetrics() const;
    
    /**
     * @brief Get transfer statistics
     * @return Transfer statistics
     */
    TransferStats getTransferStats() const;
    
    /**
     * @brief Get connection statistics
     * @return Connection statistics
     */
    ConnectionStats getConnectionStats() const;
    
    /**
     * @brief Record a transfer
     * @param bytes_transferred Bytes transferred
     * @param success Whether transfer was successful
     * @param duration_ms Transfer duration in milliseconds
     */
    void recordTransfer(uint64_t bytes_transferred, bool success, uint64_t duration_ms);
    
    /**
     * @brief Record a connection
     * @param success Whether connection was successful
     */
    void recordConnection(bool success);
    
    /**
     * @brief Record an error
     */
    void recordError();
    
    /**
     * @brief Record a timeout
     */
    void recordTimeout();
    
    /**
     * @brief Update active connection count
     * @param count Current active connection count
     */
    void updateActiveConnections(size_t count);
    
    /**
     * @brief Get metrics as JSON string
     * @return JSON representation of metrics
     */
    std::string getMetricsJson() const;
    
    /**
     * @brief Get health check as JSON string
     * @return JSON representation of health check
     */
    std::string getHealthCheckJson() const;
    
    /**
     * @brief Reset all metrics
     */
    void resetMetrics();

private:
    mutable std::mutex metrics_mutex_;
    ServerMetrics metrics_;
    std::chrono::steady_clock::time_point start_time_;
    
    void updateUptime() const;
    void updateSystemMetrics();
};

} // namespace simple_tftpd
