/**
 * @file security_manager.cpp
 * @brief TFTP production security features implementation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#include "simple-tftpd/production/security/manager.hpp"
#include <algorithm>
#include <filesystem>
#include <sstream>

namespace simple_tftpd {

ProductionSecurityManager::ProductionSecurityManager(std::shared_ptr<TftpConfig> config,
                                                     std::shared_ptr<Logger> logger)
    : config_(config), logger_(logger) {
    if (logger_) {
        logger_->log(LogLevel::INFO, "Production Security Manager initialized");
    }
}

ProductionSecurityManager::~ProductionSecurityManager() = default;

bool ProductionSecurityManager::validateFileAccess(const std::string& filename,
                                                   const std::string& client_address,
                                                   bool for_write) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!config_) {
        if (logger_) {
            logger_->log(LogLevel::ERROR, "Security validation failed: no configuration");
        }
        return false;
    }
    
    // Check client IP address
    if (!isClientAllowed(client_address)) {
        if (logger_) {
            logger_->log(LogLevel::WARNING, "Client not allowed: " + client_address);
        }
        return false;
    }
    
    // Check if operation is allowed
    if (for_write && !config_->isWriteEnabled()) {
        if (logger_) {
            logger_->log(LogLevel::WARNING, "Write operations are disabled");
        }
        return false;
    }
    
    if (!for_write && !config_->isReadEnabled()) {
        if (logger_) {
            logger_->log(LogLevel::WARNING, "Read operations are disabled");
        }
        return false;
    }
    
    // Validate filename
    if (!validateFilename(filename)) {
        if (logger_) {
            logger_->log(LogLevel::WARNING, "Invalid filename: " + filename);
        }
        return false;
    }
    
    // Normalize and validate path
    std::string normalized_path;
    if (!normalizePath(filename, normalized_path)) {
        if (logger_) {
            logger_->log(LogLevel::WARNING, "Path validation failed: " + filename);
        }
        return false;
    }
    
    // Check directory access
    std::string::size_type last_slash = normalized_path.find_last_of('/');
    std::string dir_path = (last_slash != std::string::npos) 
        ? normalized_path.substr(0, last_slash)
        : config_->getRootDirectory();
    
    if (!isDirectoryAllowed(dir_path)) {
        if (logger_) {
            logger_->log(LogLevel::WARNING, "Directory not allowed: " + dir_path);
        }
        return false;
    }
    
    // Check file extension
    std::string::size_type dot_pos = filename.find_last_of('.');
    std::string extension = (dot_pos == std::string::npos) 
        ? std::string() 
        : filename.substr(dot_pos + 1);
    
    if (!isExtensionAllowed(extension)) {
        if (logger_) {
            logger_->log(LogLevel::WARNING, "File extension not allowed: " + filename);
        }
        return false;
    }
    
    return true;
}

bool ProductionSecurityManager::isClientAllowed(const std::string& address) const {
    if (!config_) {
        return true; // Default allow if no config
    }
    
    const auto& allowed_clients = config_->getAllowedClients();
    if (allowed_clients.empty()) {
        return true; // No restrictions
    }
    
    // Check exact match
    for (const auto& allowed : allowed_clients) {
        if (address == allowed) {
            return true;
        }
        
        // Check CIDR notation (basic implementation)
        if (allowed.find('/') != std::string::npos) {
            // TODO: Implement CIDR matching
            // For now, check if address starts with the prefix
            std::string prefix = allowed.substr(0, allowed.find('/'));
            if (address.find(prefix) == 0) {
                return true;
            }
        }
    }
    
    return false;
}

bool ProductionSecurityManager::isDirectoryAllowed(const std::string& dir_path) const {
    if (!config_) {
        return true;
    }
    
    const auto& allowed_dirs = config_->getAllowedDirectories();
    if (allowed_dirs.empty()) {
        return true; // No restrictions
    }
    
    // Check if directory is in allowed list
    for (const auto& allowed : allowed_dirs) {
        if (dir_path == allowed || dir_path.find(allowed + "/") == 0) {
            return true;
        }
    }
    
    return false;
}

bool ProductionSecurityManager::isExtensionAllowed(const std::string& extension) const {
    if (!config_) {
        return true;
    }
    
    const auto& allowed_extensions = config_->getAllowedExtensions();
    if (allowed_extensions.empty()) {
        return true; // No restrictions
    }
    
    // Check if extension is in allowed list
    for (const auto& allowed : allowed_extensions) {
        if (extension == allowed) {
            return true;
        }
    }
    
    return false;
}

bool ProductionSecurityManager::normalizePath(const std::string& filename, std::string& normalized_path) const {
    if (!config_) {
        return false;
    }
    
    // Check for path traversal
    if (!checkPathTraversal(filename)) {
        return false;
    }
    
    // Build full path
    std::string root_dir = config_->getRootDirectory();
    std::string full_path = root_dir + "/" + filename;
    
    // Normalize path (remove double slashes, etc.)
    normalized_path.clear();
    bool last_was_slash = false;
    for (char c : full_path) {
        if (c == '/') {
            if (!last_was_slash) {
                normalized_path += c;
            }
            last_was_slash = true;
        } else {
            normalized_path += c;
            last_was_slash = false;
        }
    }
    
    // Ensure path is within root directory
    if (normalized_path.find(root_dir) != 0) {
        return false;
    }
    
    return true;
}

bool ProductionSecurityManager::isFileSizeAllowed(size_t file_size) const {
    if (!config_) {
        return true;
    }
    
    size_t max_size = config_->getMaxFileSize();
    if (max_size == 0) {
        return true; // No limit
    }
    
    return file_size <= max_size;
}

bool ProductionSecurityManager::isOverwriteAllowed(const std::string& filename) const {
    if (!config_) {
        return true;
    }
    
    return !config_->isOverwriteProtectionEnabled();
}

bool ProductionSecurityManager::reloadConfiguration() {
    std::lock_guard<std::mutex> lock(mutex_);
    // Configuration is managed externally, just log reload
    if (logger_) {
        logger_->log(LogLevel::INFO, "Security configuration reloaded");
    }
    return true;
}

bool ProductionSecurityManager::checkPathTraversal(const std::string& path) const {
    // Check for path traversal patterns
    if (path.find("..") != std::string::npos) {
        return false;
    }
    
    // Check for absolute paths
    if (path.find("/") == 0) {
        return false;
    }
    
    return true;
}

bool ProductionSecurityManager::validateFilename(const std::string& filename) const {
    const size_t TFTP_MAX_FILENAME_LENGTH = 512;
    
    if (filename.empty()) {
        return false;
    }
    
    if (filename.length() > TFTP_MAX_FILENAME_LENGTH) {
        return false;
    }
    
    return true;
}

} // namespace simple_tftpd
