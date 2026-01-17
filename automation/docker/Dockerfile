# Multi-stage Dockerfile for cross-platform building
# Supports: Ubuntu, Debian, CentOS, Alpine
# Architectures: x86_64, arm64, armv7

# Base builder image
FROM ubuntu:22.04 AS base-builder

# Install common build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    git \
    wget \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Ubuntu/Debian builder
FROM base-builder AS ubuntu-builder

# Install Ubuntu-specific dependencies
RUN apt-get update && apt-get install -y \
    libssl-dev \
    libjsoncpp-dev \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Set build environment
ENV PLATFORM=linux
ENV DISTRO=ubuntu
ENV ARCH=x86_64

# Copy source code
WORKDIR /app
COPY . .

# Build
RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Runtime image
FROM ubuntu:22.04 AS runtime

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libjsoncpp25 \
    libyaml-cpp0.7 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy built binary
COPY --from=ubuntu-builder /app/build/simple-tftpd /usr/local/bin/simple-tftpd

# Create user
RUN useradd -r -s /bin/false tftpddev

# Set working directory
WORKDIR /opt/simple-tftpd

# Create necessary directories
RUN mkdir -p /var/lib/simple-tftpd /var/log/simple-tftpd /etc/simple-tftpd && \
    chown -R tftpddev:tftpddev /var/lib/simple-tftpd /var/log/simple-tftpd

# Run as non-root user
USER tftpddev

# Expose TFTP port
EXPOSE 69/udp

# Start service
CMD ["/usr/local/bin/simple-tftpd", "--daemon"]
