# Build and run commands for Uncrash

import ".shared/common.just"
import ".shared/cpp.just"

# Default recipe - show available commands
default:
    @just --list

# Variables

transferDir := `if [ -d "$HOME/NextcloudPrivate/Transfer" ]; then echo "$HOME/NextcloudPrivate/Transfer"; else echo "$HOME/Nextcloud/Transfer"; fi`

# Configure the project with CMake
configure:
    mkdir -p build
    cd build && cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the application
build:
    mkdir -p build
    cd build && cmake .. && cmake --build .

# Clean build artifacts
clean:
    rm -rf build

# Run the application
run: build
    ./build/uncrash --debug

# Rebuild from scratch
rebuild: clean build

# Install the application (may require sudo)
install: build
    cmake --install build

# Run tests (if any)
test: build
    cd build && ctest --output-on-failure

# Build the Nix package
nix-build:
    nix-build -E '(import <nixpkgs> {}).callPackage ./package.nix {}'

# Build the Nix package using flakes (if available)
flake-build:
    nix build .#uncrash

# Install the Nix package to user profile
nix-install:
    nix-env -f . -i uncrash

# Run the Nix-built package (requires X11/Wayland display)
nix-run:
    #!/usr/bin/env bash
    set -euo pipefail
    echo "Running uncrash from nix build result..."
    if [ ! -e result/bin/uncrash ]; then
        echo "Error: result/bin/uncrash not found. Run 'just nix-build' first."
        exit 1
    fi
    if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
        echo "Warning: No DISPLAY or WAYLAND_DISPLAY environment variable set."
        echo "The application requires a graphical display to run."
        echo "If you're running this on a server, you may need to use Xvfb or run it on a system with a display."
    fi
    exec result/bin/uncrash --debug

# Show the build result
nix-result:
    @ls -la result/bin/

# Verify the executable was built correctly
nix-check:
    #!/usr/bin/env bash
    if [ ! -e result/bin/uncrash ]; then
        echo "❌ result/bin/uncrash not found. Run 'just nix-build' first."
        exit 1
    fi
    echo "✓ Executable exists: result/bin/uncrash"
    file result/bin/uncrash
    echo ""
    echo "✓ Checking library dependencies..."
    if ldd result/bin/uncrash | grep -i "not found" > /dev/null 2>&1; then
        echo "❌ Missing libraries detected:"
        ldd result/bin/uncrash | grep "not found"
        exit 1
    else
        echo "✓ All libraries found"
    fi
    echo ""
    echo "✓ Build appears successful!"

# Apply a git patch to the project
[group('patches')]
git-apply-patch:
    git apply {{ transferDir }}/uncrash.patch

# Create git patches for the project
[group('patches')]
git-create-patch:
    @echo "transferDir: {{ transferDir }}"
    git diff --no-ext-diff --staged --binary > {{ transferDir }}/uncrash.patch
    ls -l1t {{ transferDir }} | head -2
