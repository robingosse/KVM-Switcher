#!/usr/bin/env python3
import argparse
import sys
import time

import serial
from serial.tools import list_ports


def find_port():
    candidates = []
    for port in list_ports.comports():
        text = " ".join(filter(None, [port.description, port.manufacturer, port.product])).lower()
        if "rp2040" in text or "pico" in text or "raspberry pi" in text:
            candidates.append(port.device)

    if len(candidates) == 1:
        return candidates[0]
    if not candidates:
        raise SystemExit("No RP2040 serial device found; use --port")
    raise SystemExit(f"Multiple RP2040 serial devices found: {', '.join(candidates)}; use --port")


parser = argparse.ArgumentParser(description="Switch KVM input through the RP2040-Zero controller")
parser.add_argument("input", type=int, choices=range(1, 5), metavar="{1,2,3,4}")
parser.add_argument("--port", help="serial port, e.g. /dev/ttyACM0 or COM4")
args = parser.parse_args()

port = args.port or find_port()

with serial.Serial(port, 115200, timeout=1) as ser:
    time.sleep(0.05)
    ser.reset_input_buffer()
    ser.write(f"{args.input}\n".encode("ascii"))
    ser.flush()
    reply = ser.readline().decode("ascii", errors="replace").strip()

if reply and not reply.startswith("OK"):
    print(reply, file=sys.stderr)
    raise SystemExit(1)

print(f"KVM input {args.input}")
