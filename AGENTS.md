# GitHub Copilot Instructions for Uncrash

## Project Overview

Uncrash is a **system protection daemon** that prevents desktop crashes caused by GPU power spikes and VRM overheating. It uses a **service-based architecture** with:

- **Daemon**: Background systemd service (`uncrashd`) that runs as root
- **GUI Client**: Lightweight Kirigami interface for monitoring/configuration
- **DBus Communication**: Client-daemon communication via `org.uncrash.Daemon`

**Technology Stack:**

- **Language**: C++ (C++17)
- **Build System**: CMake + Ninja
- **GUI Framework**: Qt6 (QML/QtQuick) + KDE Kirigami
- **IPC**: Qt DBus for daemon-client communication
- **System Integration**: systemd service

## Project Structure

```
src/
  daemon/              # Background daemon (uncrashd)
    main.cpp          # Daemon entry point
    daemonservice.h/cpp  # DBus service implementation
  client/              # DBus client for GUI
    daemonclient.h/cpp   # Client interface to daemon
  powermonitor.h/cpp   # GPU power monitoring (NVIDIA/AMD)
  cpucontroller.h/cpp  # CPU frequency control
  systemprotector.h/cpp # Coordination logic
  settingsmanager.h/cpp # GUI preferences (window size, etc.)
  utils/cli.h/cpp      # CLI utilities (help, completions)
  main.cpp             # GUI entry point
  main.qml             # Kirigami UI
  org.uncrash.Daemon.xml # DBus interface definition
build/                 # CMake build directory (generated)
flake.nix              # Nix flake + NixOS module
package.nix            # Nix package definition
CMakeLists.txt         # CMake configuration (builds both binaries)
uncrashd.service.in    # systemd service template
justfile               # Task runner commands
```

## Architecture

**Service-Based Design:**

```
┌─────────────┐
│  uncrash    │  GUI Client (user-level)
│  (Kirigami) │  - Monitoring & configuration
└──────┬──────┘  - Communicates via DBus
       │
       │ DBus: org.uncrash.Daemon
       │
┌──────▼──────┐
│  uncrashd   │  System Daemon (root)
│  (systemd)  │  - GPU power monitoring
└──┬──────┬───┘  - CPU frequency control
   │      │      - Settings persistence
   │      │
   │      └─────► /sys/devices/system/cpu/.../scaling_max_freq
   │
   └────────────► nvidia-smi / /sys/class/drm/.../hwmon/
```

**Key Components:**

1. **uncrashd**: Runs as root systemd service, performs actual monitoring/throttling
2. **uncrash**: User-facing GUI, connects to daemon via DBus
3. **DBus Interface**: Exposes properties (GpuPower, MaxFrequency, etc.) and methods
4. **Config**: Daemon settings stored in `/etc/uncrash/uncrash.conf`

## Building and Running

### Build with Nix (Recommended)

Use **Just** commands for all build operations:

```bash
# Build the Nix package
just nix-build

# Run the Nix-built package
just nix-run

# Build using flakes
just flake-build

# Verify the build
just nix-check
```

### Build with CMake (Development)

```bash
# Configure the project
just configure

# Build the application
just build

# Run the application
just run

# Clean build artifacts
just clean

# Rebuild from scratch
just rebuild
```

### Testing

```bash
# Run tests
just test
```

## Important Build Notes

1. **Always use `just nix-build`** for building the Nix package - this ensures proper Nix environment
2. **Use `just build`** for quick development iterations with CMake
3. The application requires **X11 or Wayland** display to run (GUI application)
4. Build output goes to `build/` directory for CMake builds
5. Nix builds create a `result/` symlink to the Nix store

## Dependencies

### Runtime Dependencies

- Qt6 (Core, Gui, Qml, Quick, Widgets, DBus, Network)
- KDE Frameworks 6 (CoreAddons, I18n, Kirigami)
- System tools: nvidia-smi (for NVIDIA), coreutils, pciutils

### Build Dependencies

- CMake 3.20+
- Ninja
- Qt6 development packages
- KDE Extra CMake Modules
- pkg-config

## Code Style Guidelines

### C++ Code

- **Standard**: C++17
- **MOC**: Qt's Meta-Object Compiler enabled (CMAKE_AUTOMOC)
- **Include Guards**: Use `#pragma once`
- **Naming**:
  - Classes: PascalCase (e.g., `GitManager`)
  - Files: lowercase (e.g., `gitmanager.cpp`, `gitmanager.h`)
  - Methods: camelCase
- **Headers**: Separate `.h` and `.cpp` files

### QML Code

- Files in `src/*.qml`
- Dialog files follow pattern: `*Dialog.qml`
- Use Qt Quick Controls 2
- JavaScript utilities in `Utils.js`

### CMake

- Minimum version: 3.20
- Use `qt_add_executable` for Qt6
- Auto-generate resources with AUTORCC

## Version Management

Version is defined in `CMakeLists.txt`:

```cmake
project(uncrash VERSION 0.7.1 LANGUAGES CXX)
```

The version is also extracted by `package.nix` automatically from CMakeLists.txt.

## Nix Flake Structure

The project includes:

- **Package**: `uncrash` application
- **NixOS Module**: `nixosModules.uncrash` for system-wide configuration
- **Development Shell**: Available via `nix develop`

## Common Tasks

### Adding New C++ Source Files

**For Daemon:**

1. Create `.h` and `.cpp` files in `src/daemon/` or shared in `src/`
2. Add to `CMakeLists.txt` in the `add_executable(uncrashd ...)` section
3. If using Qt signals/slots, ensure the header has `Q_OBJECT` macro
4. Daemon has access to PowerMonitor, CpuController, SystemProtector

**For GUI Client:**

1. Create `.h` and `.cpp` files in `src/client/` or `src/`
2. Add to `CMakeLists.txt` in the `qt_add_executable(uncrash ...)` section
3. Client uses DaemonClient to communicate with daemon via DBus
4. GUI preferences stored via SettingsManager in `~/.config/uncrash/`

### Adding New QML Files

1. Create `.qml` file in `src/`
2. Add to `CMakeLists.txt` in the QML resources section
3. QML files are automatically compiled into resources
4. Access daemon properties via `daemonClient` in QML

### Modifying the DBus Interface

1. Edit `src/org.uncrash.Daemon.xml` to add properties/methods/signals
2. Update `src/daemon/daemonservice.{h,cpp}` to implement new interface
3. Update `src/client/daemonclient.{h,cpp}` to use new interface
4. Test with `just nix-build` to ensure both daemon and client compile

### Working with System Services

**Testing the Daemon:**

```bash
# Build the package
just nix-build

# Run daemon manually (for testing)
sudo result/bin/uncrashd

# View logs
journalctl -f
```

**Deploying to NixOS:**

1. Update `flake.nix` NixOS module if adding new configuration options
2. Module options should have sensible defaults
3. Test configuration with a VM or test system first

### Modifying Dependencies

1. Update `CMakeLists.txt` for CMake dependencies
   - Daemon: Qt6::Core, Qt6::DBus, Qt6::Network, KF6::CoreAddons, KF6::I18n
   - GUI: All daemon deps + Qt6::Gui, Qt6::Qml, Qt6::Quick, Qt6::Widgets, KF6::Kirigami
2. Update `package.nix` for Nix package dependencies
3. Rebuild with `just nix-build` to test Nix changes

## Git Workflow

### Creating Patches

```bash
# Create a patch from staged changes
just git-create-patch
```

### Applying Patches

```bash
# Apply a patch to the project
just git-apply-patch
```

Patches are stored in `~/NextcloudPrivate/Transfer/` or `~/Nextcloud/Transfer/`.

## Debugging

- Use `--debug` flag when running: `./build/uncrash --debug`
- Check `build/compile_commands.json` for IDE integration
- Qt Creator or CLion recommended for C++ development

## Important Files to Preserve

When making changes, be careful with:

- `version.h.in` - Template for version information
- `CMakeLists.txt` - Build configuration
- `flake.nix` - Nix flake definition
- `package.nix` - Package definition
- `justfile` - Build commands

## Qt/QML Integration

- Qt resources are auto-generated (CMAKE_AUTORCC)
- Meta types are generated in `build/meta_types/`
- QML type registrations are auto-generated
- Use `qt6.wrapQtAppsHook` in Nix for proper Qt plugin paths

## Installation

```bash
# Install via CMake (system-wide, may require sudo)
just install

# Install via Nix to user profile
just nix-install
```

## Testing the Nix Package

Always verify Nix builds:

```bash
# Build and check
just nix-build
just nix-check

# Show build result
just nix-result
```

## Notes for AI Assistants

1. **Always prefer `just` commands** over raw cmake/nix commands
2. When modifying C++ code, check if Qt MOC is needed (Q_OBJECT, signals, slots)
3. QML property bindings should use proper Qt Quick syntax
4. Nix builds are pure - don't rely on system dependencies
5. The app requires a display server - cannot run headless without Xvfb
6. Version bumps need to be done in CMakeLists.txt only
7. Use `just nix-build` to verify Nix package builds correctly after changes
8. Never commit changes when not asked!

## Service Architecture Guidelines

1. **Daemon (uncrashd)**: Contains the core protection logic
   - PowerMonitor: GPU power monitoring
   - CpuController: CPU frequency control
   - SystemProtector: Coordination between monitoring and control
   - DaemonService: DBus interface exposure

2. **GUI Client (uncrash)**: Monitoring and configuration only
   - DaemonClient: DBus client to communicate with daemon
   - SettingsManager: GUI preferences (separate from daemon settings)
   - QML UI: Display status, adjust settings via DBus

3. **Never put protection logic in the GUI** - It must remain in the daemon
4. **Always use DBus** for GUI-daemon communication, never direct calls
5. **Daemon settings** go in `/etc/uncrash/uncrash.conf`
6. **GUI settings** go in `~/.config/uncrash/` (via SettingsManager)

## DBus Interface

The daemon exposes the following via `org.uncrash.Daemon`:

**Properties:**

- `GpuPower` (double, read-only) - Current GPU power in watts
- `GpuPowerThreshold` (double, read/write) - Threshold for triggering throttle
- `CurrentMaxFrequency` (double, read-only) - Current CPU max frequency
- `MaxFrequency` (double, read/write) - Target CPU frequency when throttling
- `RegulationEnabled` (bool, read/write) - Enable CPU regulation
- `AutoProtection` (bool, read/write) - Enable automatic protection
- `ThresholdExceeded` (bool, read-only) - Whether threshold is exceeded

**Methods:**

- `ApplyFrequencyLimit()` - Manually apply CPU frequency limit
- `RemoveFrequencyLimit()` - Manually remove CPU frequency limit
- `GetStatus()` - Get all properties at once

**Signals:**

- `GpuPowerChanged(double)` - GPU power changed
- `ThresholdExceededChanged(bool)` - Threshold state changed
- `FrequencyLimitApplied(double)` - Frequency limit was applied
- `FrequencyLimitRemoved()` - Frequency limit was removed
