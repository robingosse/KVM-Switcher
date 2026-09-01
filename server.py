#!/usr/bin/env python3
import json
import signal
import sys
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

from gpiozero import OutputDevice

CONFIG_PATH = Path("/etc/kvm-switcher/config.json")


def load_config():
    with CONFIG_PATH.open() as f:
        return json.load(f)


CONFIG = load_config()
PULSE_SECONDS = float(CONFIG.get("pulse_ms", 150)) / 1000.0
TOKEN = str(CONFIG.get("token", ""))

if not TOKEN or TOKEN == "CHANGE-ME":
    raise SystemExit("Set a real token in /etc/kvm-switcher/config.json")

OUTPUTS = {
    str(name): OutputDevice(int(gpio), active_high=True, initial_value=False)
    for name, gpio in CONFIG["inputs"].items()
}


def press(name):
    output = OUTPUTS.get(str(name))
    if output is None:
        raise KeyError(name)
    output.on()
    try:
        time.sleep(PULSE_SECONDS)
    finally:
        output.off()


class Handler(BaseHTTPRequestHandler):
    def _reply(self, status, payload):
        body = json.dumps(payload).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path == "/health":
            self._reply(200, {"ok": True, "inputs": list(OUTPUTS)})
            return
        self._reply(404, {"ok": False, "error": "not found"})

    def do_POST(self):
        if self.headers.get("X-KVM-Token", "") != TOKEN:
            self._reply(401, {"ok": False, "error": "unauthorized"})
            return

        prefix = "/switch/"
        if not self.path.startswith(prefix):
            self._reply(404, {"ok": False, "error": "not found"})
            return

        name = self.path[len(prefix):]
        try:
            press(name)
        except KeyError:
            self._reply(404, {"ok": False, "error": "unknown input"})
            return

        self._reply(200, {"ok": True, "input": name})

    def log_message(self, fmt, *args):
        print(f"{self.address_string()} - {fmt % args}")


def shutdown(*_):
    for output in OUTPUTS.values():
        output.off()
        output.close()
    sys.exit(0)


signal.signal(signal.SIGTERM, shutdown)
signal.signal(signal.SIGINT, shutdown)

server = ThreadingHTTPServer((CONFIG.get("listen", "0.0.0.0"), int(CONFIG.get("port", 8765))), Handler)
print(f"KVM-Switcher listening on {server.server_address[0]}:{server.server_address[1]}")
server.serve_forever()
