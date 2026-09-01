#!/usr/bin/env python3
import argparse
import urllib.request

p = argparse.ArgumentParser()
p.add_argument("input")
p.add_argument("--host", default="kvmzero.local")
p.add_argument("--port", type=int, default=8765)
p.add_argument("--token", required=True)
a = p.parse_args()

url = f"http://{a.host}:{a.port}/switch/{a.input}"
req = urllib.request.Request(url, method="POST", headers={"X-KVM-Token": a.token})
with urllib.request.urlopen(req, timeout=2) as r:
    print(r.read().decode())
