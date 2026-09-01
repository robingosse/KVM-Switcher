# KVM-Switcher

Raspberry Pi Zero controller that electronically presses the existing input-select buttons on a KVM and exposes simple network commands for hotkey switching.

## Hardware

Do **not** connect unknown KVM button circuitry directly to Pi GPIO until measured.

Recommended interface:

`Pi GPIO -> opto/PhotoMOS/transistor -> two wires across existing KVM pushbutton`

The Pi only needs to imitate a momentary button press.

## Quick start on the Pi

```bash
sudo ./install.sh
sudo nano /etc/kvm-switcher/config.json
sudo systemctl restart kvm-switcher
```

## Client command

```bash
python3 kvmctl.py 2 --host kvmzero.local --token YOUR_TOKEN
```

Bind that command to any desktop hotkey you like.

## Configuration

Copy/edit `config.example.json`. GPIO numbers use BCM numbering. Add or remove inputs as required.

The default pulse is 150 ms.
