#!/usr/bin/env python3
"""Freeze beta.1 rules, AI profiles/choices, save format and power semantics."""
import argparse
import csv
import hashlib
import io
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / 'tests/fixtures'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--executable', type=Path, required=True)
parser.add_argument('--ai-executable', type=Path, required=True)
parser.add_argument('--records', action='store_true', help='compatibility; rules always checked')
args = parser.parse_args()
manifest = json.loads((BASE / 'frozen-engine-sha256.json').read_text())
for name, expected in manifest.items():
    actual = hashlib.sha256((ROOT / name).read_bytes()).hexdigest()
    assert actual == expected, f'frozen file changed: {name}'
golden = subprocess.check_output([str(args.executable.resolve())], text=True)
assert golden == (BASE / 'engine-golden-beta1.txt').read_text(), 'topology/moves/AI/RNG/undo/save golden changed'
choices = subprocess.check_output([str(args.ai_executable.resolve()), '--choices'], text=True)
rows = list(csv.reader(io.StringIO(choices)))
columns = [i for i, name in enumerate(rows[0]) if name != 'host_seconds']
observed = [[row[i] for i in columns] for row in rows]
with (BASE / 'ai-choices-beta1.csv').open(newline='') as source:
    expected = list(csv.reader(source))
assert observed == expected, '192 EASY/NORMAL/HARD choices, RNG, nodes/depth/beam changed'
new = (ROOT / 'src/ui/menus.c').read_text().split('lines[]={', 1)[1].split('};', 1)[0]
assert re.findall(r'"([^"\n]*)"', new) == json.loads((BASE / 'rules.json').read_text()), 'rule strings changed'
print(f'Freeze PASS: {len(manifest)} byte-identical files; all 192 AI choices/RNG/nodes/depth/beam, topology, paths, undo, v1 save and 21 rule strings preserved')
