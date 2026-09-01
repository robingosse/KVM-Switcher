# Hardware

## KVM harness — confirmed

KVM PCB: `KC-KVM403DH2` (2024-09-10)

Continuity testing established:

- switch pin 2 is the button signal
- switch pins 1, 3, and 4 are common/ground
- pressing the button shorts pin 2 to ground
- the common pins have continuity to PCB GND

Canonical harness:

| KVM input | Harness wire |
|---|---|
| 1 | Blue |
| 2 | Yellow |
| 3 | White |
| 4 | Red |
| GND | Black |

## Controller

Controller is an `RP2040-Zero`, not a Raspberry Pi Zero Linux computer.

Canonical GPIO map:

| KVM input | Harness | RP2040 GPIO |
|---|---|---:|
| 1 | Blue | GP0 |
| 2 | Yellow | GP2 |
| 3 | White | GP4 |
| 4 | Red | GP6 |

## Isolation/interface

Four normally-open Panasonic `AQY212GS` PhotoMOS relays are used, one per KVM input.

- Manufacturer part: `AQY212GS`
- LCSC: `C719745`
- Function: 1 Form A normally-open PhotoMOS relay
- Current-limiting resistor: 330 ohm per channel

### One channel

```text
RP2040 GPx --- 330R --- PhotoMOS input LED --- RP2040 GND

KVM SWx ------------ PhotoMOS output ------------ KVM GND
```

Repeat for four channels.

The KVM black GND wire is shared only across the KVM side of the four PhotoMOS outputs.

**Do not connect RP2040 GND to KVM GND.** Isolation is intentional.

## Channel wiring

```text
GP0 -> 330R -> PhotoMOS CH1 LED    PhotoMOS CH1 output -> Blue / KVM GND
GP2 -> 330R -> PhotoMOS CH2 LED    PhotoMOS CH2 output -> Yellow / KVM GND
GP4 -> 330R -> PhotoMOS CH3 LED    PhotoMOS CH3 output -> White / KVM GND
GP6 -> 330R -> PhotoMOS CH4 LED    PhotoMOS CH4 output -> Red / KVM GND
```

## Behaviour

All GPIO outputs initialize LOW. A valid USB command (`1` through `4`) drives exactly one PhotoMOS input HIGH for 150 ms and then returns it LOW.
