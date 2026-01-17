# simple-tftpd - Automation

This directory contains all automation scripts and configuration files for setting up and managing the simple-tftpd development environment.

## Directory Structure

```
automation/
├── ansible/                  # Ansible automation
│   ├── playbook.yml         # Ansible playbook for VM setup
│   ├── playbook-build.yml   # Comprehensive build playbook for remote VMs
│   ├── inventory.ini        # Ansible inventory file
│   ├── inventory-vmware.ini # VMware Fusion VM inventory
│   ├── ansible.cfg          # Ansible configuration
│   ├── requirements.yml     # Ansible Galaxy requirements
│   ├── Makefile.vm          # Makefile for VM operations
│   ├── vagrant-boxes.yml   # Vagrant box configurations
│   ├── scripts/             # Shell scripts for VM operations
│   │   ├── vm-ssh          # SSH wrapper for VM
│   │   ├── vm-build        # Build script for VM
│   │   ├── vm-test         # Test script for VM
│   │   ├── setup-remote.sh # Remote setup script
│   │   ├── build.sh        # Build script
│   │   ├── remote-build.sh # Remote build script for VMware VMs
│   │   ├── collect-packages.sh # Collect packages from remote VMs
│   │   └── organize-packages.sh # Organize packages into centralized directory
│   └── templates/          # Configuration templates
├── ci/                      # CI/CD configuration
│   └── Jenkinsfile         # Jenkins pipeline
├── docker/                  # Docker configuration
│   ├── Dockerfile          # Docker image definition
│   └── examples/           # Docker examples
│       └── docker-compose.yml # Docker Compose example
└── vagrant/                 # Vagrant configuration
    └── Vagrantfile         # Main Vagrantfile
```

## Quick Start

### Using Docker

```bash
# Build and run with Docker Compose
cd automation/docker
docker-compose up -d

# Or from project root
docker-compose -f automation/docker/docker-compose.yml up -d
```

### Using Vagrant

```bash
# Start VM
cd automation/vagrant
vagrant up

# SSH into VM
vagrant ssh

# Build project
./automation/ansible/scripts/vm-build
```

### Using Ansible

```bash
# Run playbook
ansible-playbook -i automation/ansible/inventory.ini automation/ansible/playbook.yml
```

## CI/CD

### Jenkins

The Jenkins pipeline is located at `automation/ci/Jenkinsfile`. It supports:
- Multi-platform builds (Linux, macOS, Windows)
- Automated testing
- Static analysis
- Package generation
- Docker image building

### Travis CI

The Travis CI configuration is located at `automation/ci/.travis.yml`. It provides:
- Automated builds on push
- Multi-platform testing
- Code coverage reporting

## Docker

Docker files are located in `automation/docker/`:
- `Dockerfile` - Multi-stage build for different distributions
- `docker-compose.yml` - Development and production configurations
- `examples/` - Example Docker configurations

## Vagrant

Vagrant configuration is in `automation/vagrant/`:
- `Vagrantfile` - Main Vagrant configuration
- `virtuals/` - Multi-VM configurations for different distributions

## Ansible

Ansible automation is in `automation/ansible/`:
- `playbook.yml` - Main playbook for environment setup
- `playbook-build.yml` - Comprehensive build playbook for remote VMs (supports Ubuntu, Debian, CentOS, macOS)
- `inventory.ini` - Development host inventory
- `inventory-vmware.ini` - VMware Fusion VM inventory for build automation
- `ansible.cfg` - Ansible configuration
- `requirements.yml` - Ansible Galaxy dependencies
- `scripts/` - Helper scripts for VM operations
  - `remote-build.sh` - Build simple-tftpd on remote VMware VMs
  - `collect-packages.sh` - Collect built packages from remote VMs
  - `organize-packages.sh` - Organize packages into centralized directory
- `templates/` - Configuration templates

### Remote Build Automation

The automation system supports building on remote VMware Fusion VMs:

```bash
# Build on all VMs
cd automation/ansible/scripts
./remote-build.sh

# Build on specific VM
./remote-build.sh -l BUILD_DEB

# Build with packages
./remote-build.sh --packages

# Clean build
./remote-build.sh -c --packages

# Build specific branch
./remote-build.sh -b develop -t Debug
```

### Package Collection

After building packages on remote VMs, collect them locally:

```bash
# Collect packages from all VMs
cd automation/ansible/scripts
./collect-packages.sh
```

This will:
1. Fetch all packages (DEB, RPM, DMG, PKG, source) from remote VMs
2. Organize them in `dist/` directory structure
3. Create centralized directory at `dist/centralized/v{VERSION}/`

---

*For detailed documentation, see the individual README files in each subdirectory.*

