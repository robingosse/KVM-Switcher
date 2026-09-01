# Hardware

## KVM harness — confirmed

KVM PCB: `KC-KVM403DH2` (2024-09-10)

Each front-panel selector switch was continuity-tested:

- switch pin 2 is the button signal
- switch pins 1, 3, and 4 are common/ground
- pressing the button shorts pin 2 to ground
- the common pins have continuity to the PCB GND test point

The case harness is therefore five wires total:

| KVM input | Harness wire |
|---|---|
| 1 | Blue |
| 2 | Yellow |
| 3 | White |
| 4 | Red |
| GND | Black |

This wire map is canonical for this modified KVM.

## Isolation/interface — selected

Use four Panasonic `AQY212GS` PhotoMOS relays, one per KVM input.

- Manufacturer part: `AQY212GS`
- LCSC: `C719745`
- Package: SOP-4, 2.54 mm pitch
- Function: normally-open 1 Form A solid-state relay
- Pi and KVM remain galvanically isolated

### One channel

```text
Pi GPIO --- 330R --- PhotoMOS LED --- Pi GND

KVM SWx -------- PhotoMOS output -------- KVM GND
```

Repeat once for each of the four switch wires. The KVM black ground wire may be shared across the KVM side of all four PhotoMOS outputs.

Do **not** connect Pi ground to KVM ground.

## Default Pi GPIO map

The current repository defaults are:

| KVM input | Harness | BCM GPIO |
|---|---|---:|
| 1 | Blue | 17 |
| 2 | Yellow | 27 |
| 3 | White | 22 |
| 4 | Red | 23 |

These GPIO assignments are configurable in `config.json`; the harness colour mapping is not.

## Behaviour

A switch command turns on the corresponding PhotoMOS for 150 ms, reproducing the physical button press (signal-to-ground closure), then releases it.
