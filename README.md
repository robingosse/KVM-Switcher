# KVM-Switcher

RP2040-Zero controller that electronically presses the four existing input-select buttons on a modified 4-port KVM.

## Architecture

```text
active PC -> USB serial -> RP2040-Zero -> 4 x PhotoMOS -> KVM buttons
```

The host sends `1`, `2`, `3`, or `4`. Firmware closes the matching isolated PhotoMOS channel for 150 ms, reproducing a physical button press.

## Confirmed KVM harness

- Input 1 = Blue
- Input 2 = Yellow
- Input 3 = White
- Input 4 = Red
- KVM GND = Black

See [`HARDWARE.md`](HARDWARE.md) for the exact wiring.

## RP2040 GPIO map

- Input 1 / Blue = GP2
- Input 2 / Yellow = GP3
- Input 3 / White = GP4
- Input 4 / Red = GP5

Each GPIO drives one PhotoMOS input LED through a 330 ohm resistor. RP2040 ground and KVM ground remain isolated.

## Firmware

Firmware lives in `firmware/` and uses the Raspberry Pi Pico SDK with USB CDC serial enabled.

Build:

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
cmake -S firmware -B build
cmake --build build
```

Flash `build/kvm_switcher.uf2` while the RP2040-Zero is in BOOTSEL mode.

## Host client

```bash
python3 -m pip install -r requirements.txt
python3 kvmctl.py 2
```

`kvmctl.py` automatically looks for a Raspberry Pi RP2040 USB serial device. Use `--port /dev/ttyACM0` if auto-detection is ambiguous.

Bind `kvmctl.py 1`, `2`, `3`, and `4` to whatever desktop hotkeys you want.
