#!/bin/bash
set -euo pipefail

if [ "${EUID}" -ne 0 ]; then
  echo "Run with sudo: sudo ./install.sh"
  exit 1
fi

apt-get update
apt-get install -y python3-gpiozero

mkdir -p /opt/kvm-switcher /etc/kvm-switcher
install -m 0755 server.py /opt/kvm-switcher/server.py
install -m 0644 kvm-switcher.service /etc/systemd/system/kvm-switcher.service

if [ ! -f /etc/kvm-switcher/config.json ]; then
  install -m 0600 config.example.json /etc/kvm-switcher/config.json
  echo "Created /etc/kvm-switcher/config.json — edit GPIOs and token before starting."
fi

systemctl daemon-reload
systemctl enable kvm-switcher

echo
echo "Next: sudo nano /etc/kvm-switcher/config.json"
echo "Then: sudo systemctl restart kvm-switcher"
