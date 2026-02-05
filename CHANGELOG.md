# Uncrash Changelog

## 0.0.4

### Added

- **Comprehensive Temperature & Fan Monitoring**: Added real-time hardware monitoring with multi-vendor support
  - New `TemperatureMonitor` class for reading temperature and fan sensors
  - **Multi-GPU Support**:
    - NVIDIA: Temperature and fan speed (%) via nvidia-smi
    - AMD: Temperature and fan speed (RPM) via amdgpu hwmon driver
    - Auto-detection of GPU vendor and model name
  - **Multi-CPU Support**:
    - AMD: k10temp and zenpower drivers
    - Intel: coretemp driver
    - Reads from `/sys/class/hwmon/` interface
  - **Multi-Motherboard Support**:
    - ASUS: asus_wmi_sensors
    - Nuvoton: nct6775, nct6683
    - ITE: it87
    - Winbond: w83627ehf
    - Fintek: f71882fg
    - Auto-detects available chipset sensors
  - **Fan Speed Monitoring**:
    - CPU fan speed in RPM
    - GPU fan speed (RPM for AMD, percentage for NVIDIA)
    - Graceful handling of missing sensors
  - Updates every 2 seconds with real-time data
  - Exposed via DBus properties: `GpuTemperature`, `GpuFanSpeed`, `CpuTemperature`, `CpuFanSpeed`, `MotherboardTemperature`, `GpuVendor`, `GpuName`

- **Temperature Monitoring UI**: New "Temperature & Fan Speeds" card in main interface
  - Displays side-by-side with "Current Status" card for easy monitoring
  - **GPU Section**: Vendor, model, temperature (color-coded), fan speed
  - **CPU Section**: Temperature (color-coded), fan speed
  - **Motherboard Section**: Temperature (color-coded)
  - Color-coded temperatures:
    - GPU: Green <70°C, Orange 70-80°C, Red >80°C
    - CPU: Green <75°C, Orange 75-85°C, Red >85°C
    - Motherboard: Green <50°C, Orange 50-60°C, Red >60°C
  - Thermometer and speedometer icons for visual clarity
  - Icons use opacity transitions to prevent UI jumping

- **System Tray Icon**: Application now runs in the system tray
  - Shows window on tray icon click
  - Hide to tray instead of closing when clicking window close button
  - Right-click context menu with "Show Uncrash" and "Quit" options
  - Custom CPU chip icon design with three states:
    - Green CPU: Normal operation (no CPU throttling)
    - Red CPU: CPU frequency limit active (throttling)
    - Gray CPU: Disconnected from daemon
  - Dynamic tooltip showing GPU power and CPU frequency
  - Updates in real-time when manual throttle buttons are clicked

- **CPU Limit Status Tracking**: Added `cpuLimitApplied` property throughout the application stack
  - `CpuController` now tracks actual CPU limit state (not just threshold exceeded)
  - Exposed via DBus as `CpuLimitApplied` property
  - Synced to GUI client via `DaemonClient`
  - Enables accurate status display for manual throttle actions

- **Enhanced UI Status Display**:
  - Main status card now shows CPU limit state instead of just threshold status
  - New "CPU Limit Status" row with APPLIED/Not Applied indicator
  - GPU Threshold shows "(EXCEEDED)" badge when threshold is exceeded
  - Color-coded status indicators (red for active, green for normal)

- **Menu Bar**: Added File menu with Quit action (Ctrl+Q keyboard shortcut)

### Changed

- Window close button now hides to tray instead of quitting the application
- Tray icon and status indicators now accurately reflect CPU throttle state for both automatic and manual operations
- Application continues running in background when window is closed
- **Improved UI Layout**:
  - Status and Temperature cards positioned side-by-side (50/50 split) at top
  - Cards have matching heights for consistent appearance
  - "How It Works" card now expands to fill remaining vertical space
  - Removed unused resource files (uncrash-active.svg, uncrash-normal.svg, resources.qrc)
- **CPU Frequency Slider Behavior**: Adjusting the CPU frequency limit slider no longer immediately applies the limit
  - Slider now acts as a configuration setting rather than an immediate action
  - Limit is only applied when:
    - Auto Protection triggers (GPU power exceeds threshold)
    - User manually clicks "Apply Limit Now" button
    - A limit is already active and needs to be updated to the new frequency
  - Prevents unintentional system disruption when adjusting settings
- **Periodic CPU Frequency Monitoring**: CpuController now updates current CPU frequency every 2 seconds
  - Ensures "CPU Max Frequency" display always shows the actual current limit
  - Updates automatically when limits are applied or removed by any mechanism

### Fixed

- Fixed QML TypeError exceptions when quitting the application
- Added null safety checks to all `daemonClient` property accesses using optional chaining (`?.`)
- Added nullish coalescing operators (`??`) to provide default values when `daemonClient` is null
- Added explicit null checks in all event handlers before calling `daemonClient` methods
- Fixed boolean type error in "Apply Limit Now" button enabled state during shutdown
- Tray icon now updates correctly when manual throttle buttons are clicked
- Status display accurately reflects whether CPU limit is actually applied
- Icon state is no longer confused between "threshold exceeded" and "limit applied"
- Fixed UI jumping when fan speed icons appear by using opacity instead of visibility
- Fixed card height calculation to prevent content from being cut off
- Fixed "CPU Max Frequency" displaying incorrect value when limit is applied
  - Now properly reflects the actual current CPU frequency limit from sysfs

## 0.0.3

### Fixed

- Fixed Kirigami.Card layout issue where content was cut off on initial app launch
- Wrapped card ColumnLayouts in Item containers with proper implicitHeight/implicitWidth bindings
- Removed conflicting anchors.fill constraints that prevented proper size calculation
- Cards now properly calculate and display their full height based on content

### Changed

- Increased default window height from 600px to 800px for better visibility
- Increased minimum window height from 400px to 700px to ensure all UI elements are visible

## 0.0.2

### Fixed

- Fixed CPU frequency control in systemd daemon by replacing interactive privilege escalation (pkexec/sudo) with direct sysfs file writes
- Daemon now properly writes to `/sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq` as root
- Improved error reporting with per-CPU success/failure messages

## 0.0.1

- GPU power monitoring (NVIDIA via nvidia-smi, AMD via sysfs)
- Automatic CPU frequency throttling when GPU power exceeds configurable threshold
- Real-time monitoring GUI with live power and frequency updates
- Manual throttle control (apply/remove frequency limits)
- Auto-protection mode for hands-free operation
- Background daemon (`uncrashd`) runs as systemd service
- Lightweight Kirigami-based GUI client (`uncrash`)
- DBus communication between daemon and client
- Declarative NixOS module with `services.uncrash` options
- Automatic DBus policy and systemd service installation
- Security hardening with ProtectSystem, ProtectHome, NoNewPrivileges
- Requires root privileges for daemon (CPU frequency control)
- Requires nvidia-smi for NVIDIA GPU support or sysfs hwmon for AMD
