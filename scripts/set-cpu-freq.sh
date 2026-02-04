#!/usr/bin/env bash
# Helper script to set CPU frequency with proper privileges
# This script should be installed with appropriate permissions

set -euo pipefail

if [ $# -ne 1 ]; then
  echo "Usage: $0 <frequency_in_khz>"
  exit 1
fi

FREQUENCY_KHZ="$1"

# Write to all CPU cores
for cpu in /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_max_freq; do
  if [ -w "$cpu" ]; then
    echo "$FREQUENCY_KHZ" >"$cpu"
  else
    echo "Warning: Cannot write to $cpu (not writable)" >&2
  fi
done

echo "CPU frequency limit set to ${FREQUENCY_KHZ} KHz"
