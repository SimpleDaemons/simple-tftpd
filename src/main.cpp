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

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <signal.h>
#include <csignal>
#include "simple_tftpd/tftp_server.hpp"
#include "simple_tftpd/tftp_config.hpp"
#include "simple_tftpd/logger.hpp"

using namespace simple_tftpd;

// Global variables for signal handling
std::shared_ptr<TftpServer> g_server;
std::shared_ptr<Logger> g_logger;
std::atomic<bool> g_shutdown_requested(false);

/**
 * @brief Signal handler for graceful shutdown
 * @param signal Signal number
 */
void signalHandler(int signal) {
    if (g_shutdown_requested.exchange(true)) {
        // Already shutting down, force exit
        std::exit(1);
    }
    
    g_logger->info("Received signal " + std::to_string(signal) + ", initiating graceful shutdown");
    
    if (g_server) {
        g_server->stop();
    }
}

/**
 * @brief Print usage information
 */
void printUsage() {
    std::cout << "\nUsage: simple-tftpd [OPTIONS] [COMMAND] [ARGS...]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  --help, -h           Show this help message" << std::endl;
    std::cout << "  --version, -v        Show version information" << std::endl;
    std::cout << "  --config, -c FILE    Use specified configuration file" << std::endl;
    std::cout << "  --verbose, -V        Enable verbose logging" << std::endl;
    std::cout << "  --daemon, -d         Run as daemon" << std::endl;
    std::cout << "  --foreground, -f     Run in foreground" << std::endl;
    std::cout << "  --test-config        Test configuration file" << std::endl;
    std::cout << "  --validate           Validate configuration" << std::endl;
    std::cout << "  --listen ADDR        Listen on specific address" << std::endl;
    std::cout << "  --port PORT          Listen on specific port" << std::endl;
    std::cout << "  --root DIR           Set root directory for file operations" << std::endl;
    
    std::cout << "\nCommands:" << std::endl;
    std::cout << "  start                Start the TFTP server" << std::endl;
    std::cout << "  stop                 Stop the TFTP server" << std::endl;
    std::cout << "  restart              Restart the TFTP server" << std::endl;
    std::cout << "  status               Show server status" << std::endl;
    std::cout << "  reload               Reload configuration" << std::endl;
    std::cout << "  test                 Test server configuration" << std::endl;
    std::cout << "  stats                Show server statistics" << std::endl;
    std::cout << "  connections          List active connections" << std::endl;
    
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  simple-tftpd start --config /etc/simple-tftpd/config.json" << std::endl;
    std::cout << "  simple-tftpd start --listen 0.0.0.0 --port 69 --root /var/tftp" << std::endl;
    std::cout << "  simple-tftpd --daemon start" << std::endl;
    std::cout << "  simple-tftpd status" << std::endl;
}

/**
 * @brief Print version information
 */
void printVersion() {
    std::cout << "simple-tftpd v0.2.0-alpha" << std::endl;
    std::cout << "Simple TFTP Daemon for Linux, macOS, and Windows" << std::endl;
    std::cout << "Copyright (c) 2024 SimpleDaemons" << std::endl;
    std::cout << "Licensed under Apache License 2.0" << std::endl;
}

/**
 * @brief Parse command line arguments
 * @param argc Argument count
 * @param argv Argument vector
 * @param config Configuration object to populate
 * @return true if parsed successfully, false otherwise
 */
bool parseArguments(int argc, char* argv[], std::shared_ptr<TftpConfig>& config) {
    std::string config_file;
    bool run_daemon = false;
    bool run_foreground = false;
    bool test_config = false;
    bool validate_config = false;
    std::string command;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return false;
        } else if (arg == "--version" || arg == "-v") {
            printVersion();
            return false;
        } else if (arg == "--config" || arg == "-c") {
            if (++i < argc) {
                config_file = argv[i];
            } else {
                std::cerr << "Error: --config requires a filename" << std::endl;
                return false;
            }
        } else if (arg == "--verbose" || arg == "-V") {
            config->setLogLevel(LogLevel::DEBUG);
        } else if (arg == "--daemon" || arg == "-d") {
            run_daemon = true;
        } else if (arg == "--foreground" || arg == "-f") {
            run_foreground = true;
        } else if (arg == "--test-config") {
            test_config = true;
        } else if (arg == "--validate") {
            validate_config = true;
        } else if (arg == "--listen") {
            if (++i < argc) {
                config->setListenAddress(argv[i]);
            } else {
                std::cerr << "Error: --listen requires an address" << std::endl;
                return false;
            }
        } else if (arg == "--port") {
            if (++i < argc) {
                try {
                    port_t port = static_cast<port_t>(std::stoi(argv[i]));
                    config->setListenPort(port);
                } catch (const std::exception& e) {
                    std::cerr << "Error: Invalid port number: " << argv[i] << std::endl;
                    return false;
                }
            } else {
                std::cerr << "Error: --port requires a port number" << std::endl;
                return false;
            }
        } else if (arg == "--root") {
            if (++i < argc) {
                config->setRootDirectory(argv[i]);
            } else {
                std::cerr << "Error: --root requires a directory" << std::endl;
                return false;
            }
        } else if (arg[0] != '-') {
            // This is a command
            command = arg;
            break;
        } else {
            std::cerr << "Error: Unknown option: " << arg << std::endl;
            printUsage();
            return false;
        }
    }
    
    // Load configuration file if specified
    if (!config_file.empty()) {
        if (!config->loadFromFile(config_file)) {
            std::cerr << "Error: Failed to load configuration file: " << config_file << std::endl;
            return false;
        }
    }
    
    // Handle special commands
    if (test_config) {
        if (config->validate()) {
            std::cout << "Configuration is valid" << std::endl;
        } else {
            std::cout << "Configuration is invalid" << std::endl;
        }
        return false;
    }
    
    if (validate_config) {
        if (config->validate()) {
            std::cout << "Configuration validation passed" << std::endl;
        } else {
            std::cout << "Configuration validation failed" << std::endl;
        }
        return false;
    }
    
    return true;
}

/**
 * @brief Initialize signal handlers
 */
void initializeSignalHandlers() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
#ifdef SIGQUIT
    signal(SIGQUIT, signalHandler);
#endif
    
#ifdef SIGHUP
    signal(SIGHUP, signalHandler);
#endif
}

/**
 * @brief Main function
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    try {
        // Create default configuration
        auto config = std::make_shared<TftpConfig>();
        
        // Parse command line arguments
        if (!parseArguments(argc, argv, config)) {
            return 0;
        }
        
        // Create logger
        g_logger = std::make_shared<Logger>(
            config->getLogFile(),
            config->getLogLevel(),
            config->isConsoleLoggingEnabled()
        );
        
        g_logger->info("Starting simple-tftpd v0.2.0-alpha");
        g_logger->info("Configuration loaded successfully");
        
        // Initialize signal handlers
        initializeSignalHandlers();
        
        // Create TFTP server
        g_server = std::make_shared<TftpServer>(config, g_logger);
        
        // Set connection callback
        g_server->setConnectionCallback([](TftpConnectionState state, const std::string& message) {
            g_logger->info("Connection state change: " + message);
        });
        
        // Set server callback
        g_server->setServerCallback([](const std::string& event, const std::string& message) {
            g_logger->info("Server event: " + event + " - " + message);
        });
        
        // Start server
        if (!g_server->start()) {
            g_logger->error("Failed to start TFTP server");
            return 1;
        }
        
        g_logger->info("TFTP server started successfully");
        g_logger->info("Listening on " + config->getListenAddress() + ":" + std::to_string(config->getListenPort()));
        g_logger->info("Root directory: " + config->getRootDirectory());
        
        // Main server loop
        while (!g_shutdown_requested && g_server->isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Shutdown
        g_logger->info("Shutting down TFTP server");
        g_server->stop();
        g_logger->info("TFTP server stopped");
        
        return 0;
        
    } catch (const std::exception& e) {
        if (g_logger) {
            g_logger->fatal("Fatal error: " + std::string(e.what()));
        } else {
            std::cerr << "Fatal error: " << e.what() << std::endl;
        }
        return 1;
    } catch (...) {
        if (g_logger) {
            g_logger->fatal("Unknown fatal error occurred");
        } else {
            std::cerr << "Unknown fatal error occurred" << std::endl;
        }
        return 1;
    }
}
