# Uncrash Changelog

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
