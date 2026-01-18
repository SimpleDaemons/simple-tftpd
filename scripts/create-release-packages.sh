#!/bin/bash
# Create Release Packages Script for simple-tftpd
# Builds packages for each version tag and creates GitHub releases

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ANSIBLE_DIR="$PROJECT_ROOT/automation/ansible"
INVENTORY_FILE="$ANSIBLE_DIR/inventory-vmware.ini"
REMOTE_BUILD_SCRIPT="$ANSIBLE_DIR/scripts/remote-build.sh"
COLLECT_SCRIPT="$ANSIBLE_DIR/scripts/collect-packages.sh"
DIST_DIR="$PROJECT_ROOT/dist"
REPO_NAME="SimpleDaemons/simple-tftpd"

# Print functions
print_header() {
    echo -e "${BLUE}╔══════════════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║         Create Release Packages for simple-tftpd Version Tags              ║${NC}"
    echo -e "${BLUE}╚══════════════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_step() {
    echo -e "${CYAN}▶ $1${NC}"
}

# Check prerequisites
check_prerequisites() {
    print_step "Checking prerequisites..."
    
    if ! command -v ansible-playbook &> /dev/null; then
        print_error "ansible-playbook is not installed"
        exit 1
    fi
    
    if ! command -v gh &> /dev/null; then
        print_error "GitHub CLI (gh) is not installed"
        print_info "Install with: brew install gh"
        exit 1
    fi
    
    if ! gh auth status &> /dev/null; then
        print_error "GitHub CLI is not authenticated"
        print_info "Run: gh auth login"
        exit 1
    fi
    
    if [ ! -f "$INVENTORY_FILE" ]; then
        print_error "Inventory file not found: $INVENTORY_FILE"
        exit 1
    fi
    
    if [ ! -f "$REMOTE_BUILD_SCRIPT" ]; then
        print_error "Remote build script not found: $REMOTE_BUILD_SCRIPT"
        exit 1
    fi
    
    print_success "Prerequisites check passed"
}

# Get version tags
get_version_tags() {
    print_step "Getting version tags..."
    
    cd "$PROJECT_ROOT"
    TAGS=$(git tag -l | grep -E "^v[0-9]+\.[0-9]+\.[0-9]+" | sort -V)
    
    if [ -z "$TAGS" ]; then
        print_error "No version tags found"
        exit 1
    fi
    
    echo "$TAGS"
}

# Get release notes for a tag
get_release_notes() {
    local tag=$1
    local notes=""
    
    # Get tag message
    notes=$(git tag -l -n1 "$tag" | sed "s/^$tag[[:space:]]*//")
    
    # If no notes, create default
    if [ -z "$notes" ]; then
        notes="Release $tag"
    fi
    
    echo "$notes"
}

# Build packages for a version tag
build_packages_for_tag() {
    local tag=$1
    local version=${tag#v}  # Remove 'v' prefix
    
    print_step "Building packages for $tag..."
    
    cd "$PROJECT_ROOT"
    
    # Checkout the tag
    print_info "Checking out tag: $tag"
    git checkout "$tag" 2>/dev/null || {
        print_error "Failed to checkout tag: $tag"
        return 1
    }
    
    # Update CMakeLists.txt version if needed (should already be correct in tag)
    print_info "Version in CMakeLists.txt: $(grep 'VERSION' CMakeLists.txt | head -1)"
    
    # Run remote builds on all platforms
    print_info "Building on all platforms..."
    
    # Build on Debian (DEB packages)
    print_info "Building DEB packages on BUILD_DEB..."
    if bash "$REMOTE_BUILD_SCRIPT" \
        -b "$tag" \
        -t Release \
        -c \
        --packages \
        -l BUILD_DEB; then
        print_success "DEB packages built successfully"
    else
        print_warning "DEB build failed, continuing..."
    fi
    
    # Build on RHEL/CentOS (RPM packages)
    print_info "Building RPM packages on BUILD_RPM..."
    if bash "$REMOTE_BUILD_SCRIPT" \
        -b "$tag" \
        -t Release \
        -c \
        --packages \
        -l BUILD_RPM; then
        print_success "RPM packages built successfully"
    else
        print_warning "RPM build failed, continuing..."
    fi
    
    # Build on macOS (DMG/PKG packages)
    print_info "Building macOS packages on BUILD_MACOS..."
    if bash "$REMOTE_BUILD_SCRIPT" \
        -b "$tag" \
        -t Release \
        -c \
        --packages \
        -l BUILD_MACOS; then
        print_success "macOS packages built successfully"
    else
        print_warning "macOS build failed, continuing..."
    fi
    
    # Collect packages
    print_info "Collecting packages..."
    if bash "$COLLECT_SCRIPT"; then
        print_success "Packages collected successfully"
    else
        print_warning "Package collection had issues, continuing..."
    fi
    
    # Return to main branch
    git checkout main 2>/dev/null || git checkout master 2>/dev/null || true
    
    return 0
}

# Create GitHub release
create_github_release() {
    local tag=$1
    local version=${tag#v}
    local notes=$(get_release_notes "$tag")
    local release_dir="$DIST_DIR/centralized/v$version"
    
    print_step "Creating GitHub release for $tag..."
    
    # Check if release already exists
    if gh release view "$tag" --repo "$REPO_NAME" &> /dev/null; then
        print_warning "Release $tag already exists, skipping creation"
        print_info "To update, delete the release first: gh release delete $tag --repo $REPO_NAME"
        return 0
    fi
    
    # Prepare release notes file
    local notes_file=$(mktemp)
    cat > "$notes_file" << EOF
$notes

## Packages

This release includes packages for:
- **Linux (Debian/Ubuntu)**: DEB packages
- **Linux (RHEL/CentOS/Fedora)**: RPM packages  
- **macOS**: DMG and PKG packages
- **Source**: Source tarballs and ZIP archives

## Installation

### Debian/Ubuntu
\`\`\`bash
sudo dpkg -i simple-tftpd_${version}_*.deb
\`\`\`

### RHEL/CentOS/Fedora
\`\`\`bash
sudo rpm -ivh simple-tftpd-${version}-*.rpm
\`\`\`

### macOS
Download and open the DMG file, or install via:
\`\`\`bash
sudo installer -pkg simple-tftpd-${version}-*.pkg -target /
\`\`\`

## Source Code
\`\`\`bash
git clone https://github.com/$REPO_NAME.git
cd simple-tftpd
git checkout $tag
\`\`\`
EOF
    
    # Create release (draft first, then we'll upload assets)
    print_info "Creating release draft..."
    if gh release create "$tag" \
        --repo "$REPO_NAME" \
        --title "$tag" \
        --notes-file "$notes_file" \
        --draft; then
        print_success "Release draft created"
    else
        print_error "Failed to create release"
        rm -f "$notes_file"
        return 1
    fi
    
    # Upload packages
    if [ -d "$release_dir" ]; then
        print_info "Uploading packages from $release_dir..."
        local uploaded=0
        
        # Use find to locate packages, handling cases where no files match
        shopt -s nullglob
        for package in "$release_dir"/*.{deb,rpm,dmg,pkg} "$release_dir"/*.tar.gz "$release_dir"/*.zip; do
            if [ -f "$package" ]; then
                print_info "Uploading: $(basename "$package")"
                if gh release upload "$tag" "$package" --repo "$REPO_NAME" --clobber; then
                    ((uploaded++))
                    print_success "Uploaded: $(basename "$package")"
                else
                    print_warning "Failed to upload: $(basename "$package")"
                fi
            fi
        done
        shopt -u nullglob
        
        if [ $uploaded -gt 0 ]; then
            print_success "Uploaded $uploaded package(s)"
        else
            print_warning "No packages found to upload"
        fi
    else
        print_warning "Package directory not found: $release_dir"
    fi
    
    # Publish the release
    print_info "Publishing release..."
    if gh release edit "$tag" --repo "$REPO_NAME" --draft=false; then
        print_success "Release published: https://github.com/$REPO_NAME/releases/tag/$tag"
    else
        print_warning "Failed to publish release (may already be published)"
    fi
    
    rm -f "$notes_file"
    return 0
}

# Process a single version tag
process_version_tag() {
    local tag=$1
    
    print_header
    print_info "Processing version tag: $tag"
    echo ""
    
    # Build packages
    if build_packages_for_tag "$tag"; then
        print_success "Packages built for $tag"
    else
        print_error "Failed to build packages for $tag"
        return 1
    fi
    
    echo ""
    
    # Create GitHub release
    if create_github_release "$tag"; then
        print_success "Release created for $tag"
    else
        print_warning "Failed to create release for $tag (packages may still be available)"
    fi
    
    echo ""
    return 0
}

# Show usage
show_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Build packages and create GitHub releases for version tags.

OPTIONS:
    -h, --help              Show this help message
    -t, --tag TAG           Build packages for a specific tag only
    -a, --all               Build packages for all version tags (default)
    -y, --yes               Skip confirmation prompts (non-interactive)
    -l, --list              List available version tags and exit

EXAMPLES:
    $0                      # Build all tags (interactive)
    $0 --all                # Build all tags (interactive)
    $0 --tag v0.2.1         # Build only v0.2.1
    $0 --all --yes          # Build all tags without prompts
    $0 --list               # List available tags

EOF
}

# Main function
main() {
    local SELECTED_TAG=""
    local BUILD_ALL=true
    local SKIP_CONFIRM=false
    local LIST_ONLY=false
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_usage
                exit 0
                ;;
            -t|--tag)
                SELECTED_TAG="$2"
                BUILD_ALL=false
                shift 2
                ;;
            -a|--all)
                BUILD_ALL=true
                shift
                ;;
            -y|--yes)
                SKIP_CONFIRM=true
                shift
                ;;
            -l|--list)
                LIST_ONLY=true
                shift
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done
    
    print_header
    
    check_prerequisites
    echo ""
    
    # Get version tags
    TAGS=$(get_version_tags)
    
    if [ -z "$TAGS" ]; then
        print_error "No version tags found"
        exit 1
    fi
    
    # List tags and exit if requested
    if [ "$LIST_ONLY" = true ]; then
        print_info "Available version tags:"
        echo "$TAGS" | while read tag; do
            echo "  - $tag"
        done
        exit 0
    fi
    
    # Determine which tags to process
    if [ "$BUILD_ALL" = false ] && [ -n "$SELECTED_TAG" ]; then
        # Verify the selected tag exists
        if ! echo "$TAGS" | grep -q "^${SELECTED_TAG}$"; then
            print_error "Tag '$SELECTED_TAG' not found"
            print_info "Available tags:"
            echo "$TAGS" | while read tag; do
                echo "  - $tag"
            done
            exit 1
        fi
        TAGS_TO_BUILD="$SELECTED_TAG"
        print_info "Building packages for tag: $SELECTED_TAG"
    else
        TAGS_TO_BUILD="$TAGS"
        print_info "Found version tags:"
        echo "$TAGS_TO_BUILD" | while read tag; do
            echo "  - $tag"
        done
    fi
    echo ""
    
    # Ask for confirmation unless --yes is used
    if [ "$SKIP_CONFIRM" = false ]; then
        if [ "$BUILD_ALL" = true ]; then
            read -p "Build packages and create releases for all tags? (y/N): " -n 1 -r
        else
            read -p "Build packages and create release for tag '$SELECTED_TAG'? (y/N): " -n 1 -r
        fi
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_info "Cancelled by user"
            exit 0
        fi
    fi
    
    # Process each tag
    local success_count=0
    local fail_count=0
    local total_tags=$(echo "$TAGS_TO_BUILD" | wc -l | tr -d ' ')
    local current=0
    
    while IFS= read -r tag; do
        ((current++))
        print_info "Processing tag $current of $total_tags: $tag"
        
        if process_version_tag "$tag"; then
            ((success_count++))
        else
            ((fail_count++))
        fi
        
        if [ $current -lt $total_tags ] && [ "$SKIP_CONFIRM" = false ]; then
            echo ""
            print_info "Press Enter to continue to next tag, or Ctrl+C to stop..."
            read
        fi
    done <<< "$TAGS_TO_BUILD"
    
    echo ""
    print_header
    print_success "Release creation completed!"
    print_info "Success: $success_count, Failed: $fail_count"
}

# Run main function
main "$@"
