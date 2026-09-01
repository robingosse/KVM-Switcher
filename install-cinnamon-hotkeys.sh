#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PY="$ROOT/.venv/bin/python"
CTL="$ROOT/kvmctl.py"

if [[ ! -x "$PY" ]]; then
  echo "Missing virtualenv at $ROOT/.venv"
  echo "Run: python3 -m venv .venv && .venv/bin/pip install -r requirements.txt"
  exit 1
fi

if [[ ! -f "$CTL" ]]; then
  echo "Missing $CTL"
  exit 1
fi

python3 - "$ROOT" <<'PY'
import ast
import subprocess
import sys

root = sys.argv[1]
base_schema = "org.cinnamon.desktop.keybindings"
custom_schema = "org.cinnamon.desktop.keybindings.custom-keybinding"
base_path = "/org/cinnamon/desktop/keybindings/custom-keybindings/{}/"
ids = [f"kvm-switcher-{i}" for i in range(1, 5)]

current_raw = subprocess.check_output(
    ["gsettings", "get", base_schema, "custom-list"], text=True
).strip()
try:
    current = ast.literal_eval(current_raw)
except Exception:
    current = []

for item in ids:
    if item not in current:
        current.append(item)

subprocess.run(
    ["gsettings", "set", base_schema, "custom-list", repr(current)],
    check=True,
)

for i, item in enumerate(ids, start=1):
    schema = f"{custom_schema}:{base_path.format(item)}"
    command = f"{root}/.venv/bin/python {root}/kvmctl.py {i}"
    binding = f"['<Primary><Alt>{i}']"
    subprocess.run(["gsettings", "set", schema, "name", f"KVM Input {i}"], check=True)
    subprocess.run(["gsettings", "set", schema, "command", command], check=True)
    subprocess.run(["gsettings", "set", schema, "binding", binding], check=True)

print("Installed Cinnamon hotkeys:")
for i in range(1, 5):
    print(f"  Ctrl+Alt+{i} -> KVM input {i}")
PY

echo
echo "If the shortcuts do not work yet, log out and back in once so Cinnamon inherits your dialout group membership."
