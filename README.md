# KVM-Switcher

Raspberry Pi Zero controller that electronically presses the existing input-select buttons on a 4-port KVM and exposes simple network commands for hotkey switching.

## Confirmed KVM wiring

The selector buttons short their signal pin to KVM ground when pressed.

Harness mapping:

- Input 1 = Blue
- Input 2 = Yellow
- Input 3 = White
- Input 4 = Red
- KVM GND = Black

Interface: four isolated Panasonic `AQY212GS` PhotoMOS relays (LCSC `C719745`), one per input. Do **not** join Pi GND to KVM GND.

See [`HARDWARE.md`](HARDWARE.md) for the exact wiring and default GPIO map.

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

Copy/edit `config.example.json`. GPIO numbers use BCM numbering.

Default GPIOs are 17, 27, 22, and 23 for KVM inputs 1 through 4. The default virtual button press is 150 ms.
