/**
 * @file security_manager.hpp
 * @brief TFTP production security features including access control, filtering, and validation
 * @author SimpleDaemons
 * @copyright 2024 SimpleDaemons
 * @license Apache-2.0
 */

#ifndef SIMPLE_TFTPD_PRODUCTION_SECURITY_MANAGER_HPP
#define SIMPLE_TFTPD_PRODUCTION_SECURITY_MANAGER_HPP

#include "simple-tftpd/core/utils/logger.hpp"
#include "simple-tftpd/core/config/config.hpp"
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <set>

namespace simple_tftpd {

/**
 * @brief Production security manager for TFTP daemon
 *
 * Handles production-specific security features:
 * - Directory access control
 * - File extension filtering
 * - IP address filtering
 * - Path traversal protection
 * - Overwrite protection
 * - File size limits
 * - Read/write permission enforcement
 */
class ProductionSecurityManager {
public:
    /**
     * @brief Constructor
     * @param config Server configuration
     * @param logger Logger instance
     */
    ProductionSecurityManager(std::shared_ptr<TftpConfig> config,
                             std::shared_ptr<Logger> logger);

    /**
     * @brief Destructor
     */
    ~ProductionSecurityManager();

    /**
     * @brief Validate file access for a request
     * @param filename Filename to validate
     * @param client_address Client IP address
     * @param for_write Whether this is a write operation
     * @return true if access is allowed, false otherwise
     */
    bool validateFileAccess(const std::string& filename,
                           const std::string& client_address,
                           bool for_write) const;

    /**
     * @brief Check if client address is allowed
     * @param address Client IP address
     * @return true if allowed, false otherwise
     */
    bool isClientAllowed(const std::string& address) const;

    /**
     * @brief Check if directory is allowed
     * @param dir_path Directory path
     * @return true if allowed, false otherwise
     */
    bool isDirectoryAllowed(const std::string& dir_path) const;

    /**
     * @brief Check if file extension is allowed
     * @param extension File extension (without dot)
     * @return true if allowed, false otherwise
     */
    bool isExtensionAllowed(const std::string& extension) const;

    /**
     * @brief Validate and normalize file path
     * @param filename Input filename
     * @param normalized_path Output normalized path
     * @return true if valid, false otherwise
     */
    bool normalizePath(const std::string& filename, std::string& normalized_path) const;

    /**
     * @brief Check if file size is within limits
     * @param file_size File size in bytes
     * @return true if within limits, false otherwise
     */
    bool isFileSizeAllowed(size_t file_size) const;

    /**
     * @brief Check if overwrite is allowed
     * @param filename Filename to check
     * @return true if overwrite allowed, false otherwise
     */
    bool isOverwriteAllowed(const std::string& filename) const;

    /**
     * @brief Reload security configuration
     * @return true if reloaded successfully
     */
    bool reloadConfiguration();

private:
    std::shared_ptr<TftpConfig> config_;
    std::shared_ptr<Logger> logger_;
    mutable std::mutex mutex_;

    /**
     * @brief Check for path traversal attacks
     * @param path Path to check
     * @return true if safe, false if traversal detected
     */
    bool checkPathTraversal(const std::string& path) const;

    /**
     * @brief Validate filename format
     * @param filename Filename to validate
     * @return true if valid, false otherwise
     */
    bool validateFilename(const std::string& filename) const;
};

} // namespace simple_tftpd

#endif // SIMPLE_TFTPD_PRODUCTION_SECURITY_MANAGER_HPP
