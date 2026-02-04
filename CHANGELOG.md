# Uncrash Changelog

## 0.0.3

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
