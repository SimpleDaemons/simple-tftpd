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

#include "simple-tftpd/core/tftp/monitoring.hpp"
#include <sstream>
#include <iomanip>
#include <mutex>
#include <algorithm>
#include <chrono>

namespace simple_tftpd {

Monitoring::Monitoring() : start_time_(std::chrono::steady_clock::now()) {
    metrics_.server_start_time = start_time_;
}

Monitoring::~Monitoring() = default;

HealthCheckResult Monitoring::performHealthCheck() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    HealthCheckResult result;
    
    // Basic health checks
    bool healthy = true;
    std::vector<std::string> issues;
    
    // Check if server has been running
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(
        now - metrics_.server_start_time);
    
    if (uptime.count() < 0) {
        healthy = false;
        issues.push_back("Invalid server start time");
    }
    
    // Check error rate (if we have transfers)
    if (metrics_.transfers.total_transfers > 0) {
        double error_rate = static_cast<double>(metrics_.transfers.failed_transfers) /
                           static_cast<double>(metrics_.transfers.total_transfers);
        if (error_rate > 0.5) {
            healthy = false;
            issues.push_back("High error rate: " + std::to_string(error_rate * 100) + "%");
        }
    }
    
    // Check connection failure rate
    if (metrics_.connections.total_connections > 0) {
        double failure_rate = static_cast<double>(metrics_.connections.failed_connections) /
                             static_cast<double>(metrics_.connections.total_connections);
        if (failure_rate > 0.3) {
            healthy = false;
            issues.push_back("High connection failure rate: " + 
                           std::to_string(failure_rate * 100) + "%");
        }
    }
    
    // Set status
    if (healthy && issues.empty()) {
        result.status = HealthStatus::HEALTHY;
        result.message = "Server is healthy";
    } else if (issues.size() == 1) {
        result.status = HealthStatus::DEGRADED;
        result.message = issues[0];
    } else {
        result.status = HealthStatus::UNHEALTHY;
        result.message = "Multiple issues detected";
    }
    
    // Add details
    result.details["uptime_seconds"] = std::to_string(uptime.count());
    result.details["total_transfers"] = std::to_string(metrics_.transfers.total_transfers);
    result.details["active_connections"] = std::to_string(metrics_.connections.active_connections);
    result.details["total_errors"] = std::to_string(metrics_.total_errors);
    
    return result;
}

ServerMetrics Monitoring::getMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    updateUptime();
    ServerMetrics m = metrics_;
    return m;
}

TransferStats Monitoring::getTransferStats() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return metrics_.transfers;
}

ConnectionStats Monitoring::getConnectionStats() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return metrics_.connections;
}

void Monitoring::recordTransfer(uint64_t bytes_transferred, bool success, uint64_t duration_ms) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    metrics_.transfers.total_transfers++;
    if (success) {
        metrics_.transfers.successful_transfers++;
        metrics_.transfers.total_bytes_sent += bytes_transferred;
    } else {
        metrics_.transfers.failed_transfers++;
    }
    
    // Update average transfer time
    if (metrics_.transfers.total_transfers > 0) {
        metrics_.transfers.average_transfer_time_ms = 
            (metrics_.transfers.average_transfer_time_ms * (metrics_.transfers.total_transfers - 1) +
             duration_ms) / metrics_.transfers.total_transfers;
    }
    
    metrics_.transfers.last_transfer_time = std::chrono::steady_clock::now();
}

void Monitoring::recordConnection(bool success) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    metrics_.connections.total_connections++;
    if (success) {
        metrics_.connections.last_connection_time = std::chrono::steady_clock::now();
    } else {
        metrics_.connections.failed_connections++;
    }
}

void Monitoring::recordError() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_.total_errors++;
}

void Monitoring::recordTimeout() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_.total_timeouts++;
}

void Monitoring::updateActiveConnections(size_t count) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_.connections.active_connections = count;
    if (count > metrics_.connections.peak_connections) {
        metrics_.connections.peak_connections = count;
    }
}

std::string Monitoring::getMetricsJson() const {
    auto metrics = getMetrics();
    std::ostringstream oss;
    
    oss << "{\n";
    oss << "  \"transfers\": {\n";
    oss << "    \"total\": " << metrics.transfers.total_transfers << ",\n";
    oss << "    \"successful\": " << metrics.transfers.successful_transfers << ",\n";
    oss << "    \"failed\": " << metrics.transfers.failed_transfers << ",\n";
    oss << "    \"bytes_sent\": " << metrics.transfers.total_bytes_sent << ",\n";
    oss << "    \"bytes_received\": " << metrics.transfers.total_bytes_received << ",\n";
    oss << "    \"average_time_ms\": " << metrics.transfers.average_transfer_time_ms << "\n";
    oss << "  },\n";
    oss << "  \"connections\": {\n";
    oss << "    \"total\": " << metrics.connections.total_connections << ",\n";
    oss << "    \"active\": " << metrics.connections.active_connections << ",\n";
    oss << "    \"peak\": " << metrics.connections.peak_connections << ",\n";
    oss << "    \"failed\": " << metrics.connections.failed_connections << "\n";
    oss << "  },\n";
    oss << "  \"errors\": " << metrics.total_errors << ",\n";
    oss << "  \"timeouts\": " << metrics.total_timeouts << ",\n";
    oss << "  \"uptime_seconds\": " << metrics.uptime.count() << "\n";
    oss << "}";
    
    return oss.str();
}

std::string Monitoring::getHealthCheckJson() const {
    auto health = performHealthCheck();
    std::ostringstream oss;
    
    oss << "{\n";
    oss << "  \"status\": \"";
    switch (health.status) {
        case HealthStatus::HEALTHY:
            oss << "healthy";
            break;
        case HealthStatus::DEGRADED:
            oss << "degraded";
            break;
        case HealthStatus::UNHEALTHY:
            oss << "unhealthy";
            break;
    }
    oss << "\",\n";
    oss << "  \"message\": \"" << health.message << "\",\n";
    oss << "  \"details\": {\n";
    bool first = true;
    for (const auto& [key, value] : health.details) {
        if (!first) oss << ",\n";
        oss << "    \"" << key << "\": \"" << value << "\"";
        first = false;
    }
    oss << "\n  }\n";
    oss << "}";
    
    return oss.str();
}

void Monitoring::resetMetrics() {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    metrics_ = ServerMetrics();
    metrics_.server_start_time = std::chrono::steady_clock::now();
    start_time_ = metrics_.server_start_time;
}

void Monitoring::updateUptime() const {
    auto now = std::chrono::steady_clock::now();
    metrics_.uptime = std::chrono::duration_cast<std::chrono::seconds>(
        now - metrics_.server_start_time);
}

void Monitoring::updateSystemMetrics() {
    // Placeholder for system metrics (CPU, memory)
    // Can be implemented with platform-specific code
    metrics_.cpu_usage_percent = 0.0;
    metrics_.memory_usage_bytes = 0;
}

} // namespace simple_tftpd
