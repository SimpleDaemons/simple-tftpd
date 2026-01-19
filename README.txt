Simple TFTP Daemon
=================

Version: v0.2.1
Status: Production Ready

A lightweight, cross-platform TFTP (Trivial File Transfer Protocol) daemon written in C++17. Implements RFC 1350 with full support for TFTP options (RFC 2347), windowed transfers, and comprehensive security features.

Features
--------

Core Protocol:
- RFC 1350 Compliant: Full TFTP protocol implementation
- TFTP Options (RFC 2347): blksize, timeout, tsize, windowsize negotiation
- Windowed Transfers: Sliding window with configurable window size
- Retransmission: Automatic retry with configurable max retries and timeout handling
- Transfer Modes: netascii, octet, and mail mode support

Security:
- Access Control: Directory allowlists and path traversal protection
- File Filtering: Extension allowlists and file size limits
- Client Filtering: IP address allowlists
- Permission Control: Configurable read/write permissions

Configuration:
- JSON Configuration: Full configuration file support with validation
- Configuration Serialization: Save and export configuration
- Command-line Overrides: Runtime configuration options
- Hot Reload: Reload configuration without restart (SIGHUP signal)

Platform Support:
- Cross-platform: Linux, macOS, and Windows
- IPv6 Support: Optional IPv6 support
- Modern C++: Built with C++17 standard
- Logging: Comprehensive logging with configurable levels

License
-------

Simple TFTP Daemon uses a dual-licensing model:

Production Edition (Apache 2.0) - Free & Open Source

This repository contains the Production Edition, which is licensed under the Apache License, Version 2.0. This edition includes all core TFTP protocol features, basic security, configuration management, and production monitoring - everything needed for production deployments.

License: Apache License, Version 2.0
Use: Free for any use (commercial or non-commercial)
Source: This repository

See LICENSE.txt for the full Apache 2.0 license text.

Enterprise Edition (BSL) - Commercial License

The Enterprise Edition includes advanced features such as:
- Advanced authentication and authorization
- Rate limiting and DDoS protection
- Advanced monitoring and analytics
- Priority support

For more information about the Enterprise Edition, visit:
https://github.com/simpledaemons/simple-tftpd

Installation
------------

macOS:
1. Open the DMG file
2. Drag simple-tftpd to Applications folder
3. Configure the service using the provided configuration files

Linux:
- Debian/Ubuntu: sudo dpkg -i simple-tftpd-*.deb
- Red Hat/CentOS: sudo rpm -i simple-tftpd-*.rpm

Windows:
- Run the MSI installer and follow the setup wizard

Documentation
-------------

Full documentation is available in the docs/ directory included with this package.

Quick Start:
1. Copy the example configuration file to /etc/simple-tftpd/simple-tftpd.conf
2. Edit the configuration file to set your root directory and other settings
3. Start the service:
   - macOS: sudo launchctl load /Library/LaunchDaemons/com.simpledaemons.simple-tftpd.plist
   - Linux: sudo systemctl start simple-tftpd
   - Windows: The service starts automatically after installation

Support
-------

For issues, questions, or contributions:
- GitHub: https://github.com/simpledaemons/simple-tftpd
- Email: info@simpledaemons.com

Copyright 2024 SimpleDaemons
Licensed under the Apache License, Version 2.0
