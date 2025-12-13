# Simple TFTP Daemon - Deployment Diagrams

## Basic Deployment Architecture

```mermaid
graph TB
    subgraph "Client Network"
        Client1[TFTP Client 1]
        Client2[TFTP Client 2]
        ClientN[TFTP Client N]
    end

    subgraph "TFTP Server"
        Server[simple-tftpd<br/>Main Process]
        Config[/etc/simple-tftpd/<br/>Configuration]
        RootDir[/var/tftp<br/>Root Directory]
        Logs[/var/log/simple-tftpd/<br/>Transfer Logs]
    end

    subgraph "System Services"
        Systemd[systemd<br/>Service Manager]
    end

    Client1 --> Server
    Client2 --> Server
    ClientN --> Server

    Systemd --> Server
    Systemd --> Config

    Server --> Config
    Server --> RootDir
    Server --> Logs
```

## PXE Boot Deployment

```mermaid
graph TB
    subgraph "PXE Clients"
        PXE1[PXE Client 1]
        PXE2[PXE Client 2]
        PXEN[PXE Client N]
    end

    subgraph "TFTP Server"
        Server[simple-tftpd<br/>PXE Boot Server]
        BootFiles[/var/tftp/pxelinux.0<br/>Boot Files]
        Kernel[/var/tftp/kernel<br/>Kernel Images]
        Initrd[/var/tftp/initrd<br/>Initrd Images]
    end

    subgraph "DHCP Server"
        DHCP[DHCP Server<br/>PXE Options]
    end

    PXE1 --> DHCP
    PXE2 --> DHCP
    PXEN --> DHCP

    DHCP -->|PXE Boot Info| PXE1
    DHCP -->|PXE Boot Info| PXE2
    DHCP -->|PXE Boot Info| PXEN

    PXE1 --> Server
    PXE2 --> Server
    PXEN --> Server

    Server --> BootFiles
    Server --> Kernel
    Server --> Initrd
```
