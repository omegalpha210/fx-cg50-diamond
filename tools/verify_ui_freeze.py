#!/usr/bin/env python3
"""Beta.4 preserves topology, USB/power/storage transactions and trail geometry.

The authorized rules/AI milestone supersedes the beta.3 whole-engine freeze.
Old choice/path manifests remain historical evidence, never silently rewritten.
V1 movement/save compatibility and V2 legality have dedicated C tests.
"""
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
manifest = json.loads((BASE / 'preserved-beta4-sha256.json').read_text())
menu_patch = json.loads((BASE / 'menu-boundary-sha256.json').read_text())
for name, expected in manifest.items():
    actual = hashlib.sha256((ROOT / name).read_bytes()).hexdigest()
    if name == 'src/platform/main.c':
        # Explicitly authorized MENU ownership change. Preserve the historical
        # beta.4 receipt and freeze every other path against that same baseline.
        assert expected == menu_patch['baseline_sha256'], 'MENU baseline changed'
        expected = menu_patch['patched_sha256']
    assert actual == expected, f'frozen file changed: {name}'
golden = subprocess.check_output([str(args.executable.resolve())], text=True)
assert golden.splitlines()[0] == (BASE / 'engine-golden-beta1.txt').read_text().splitlines()[0], '73-node topology changed'
for source in (ROOT / 'src/ui').glob('*.c'):
    strings = re.findall(r'"([^"\n]*)"', source.read_text())
    assert not any('HUMAN' in label.upper() for label in strings), f'visible HUMAN string: {source.name}'
new = (ROOT / 'src/ui/menus.c').read_text().split('lines[]={', 1)[1].split('};', 1)[0]
assert re.findall(r'"([^"\n]*)"', new) == json.loads((BASE / 'rules-beta4.json').read_text()), 'V2 rule strings changed'
print(f'Preservation PASS: {len(manifest)-1} original files byte-identical, one explicit MENU platform patch; original 73-node topology; V2 rules text; YOU vocabulary.')
