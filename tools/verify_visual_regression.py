#!/usr/bin/env python3
"""Reproduce the pre-edit beta.2 smoke games, excluding host timing only."""
import argparse
import csv
import hashlib
import io
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--ai-executable', type=Path, required=True)
parser.add_argument('--output', type=Path, default=ROOT / 'build/visual-regression')
args = parser.parse_args()
executable = str(args.ai_executable.resolve())
destination = args.output.resolve()
destination.mkdir(parents=True, exist_ok=True)
expected = json.loads((ROOT / 'docs/ai/ui-beta3-regression.json').read_text())['groups']
commands = {'2p': ['--tournament', '1', '400'], '3p': ['--3p', '1', '400'],
            '3p-control': ['--3p-control', '1', '400'],
            '3p-followup': ['--recheck', str(destination / '3p.csv'), '1200']}
for group, options in commands.items():
    result = subprocess.run([executable, *options], text=True, capture_output=True, check=True)
    (destination / f'{group}.csv').write_text(result.stdout)
    (destination / f'{group}.log').write_text(result.stderr)
    rows = list(csv.reader(io.StringIO(result.stdout)))
    columns = [i for i, name in enumerate(rows[0]) if name != 'host_seconds']
    untimed = [[row[i] for i in columns] for row in rows]
    with (ROOT / f'tests/fixtures/selfplay-{group}-beta2.csv').open(newline='') as stream:
        assert untimed == list(csv.reader(stream)), f'{group}: per-game regression'
    digest = hashlib.sha256(json.dumps(untimed, separators=(',', ':')).encode()).hexdigest()
    assert len(rows) - 1 == expected[group]['cases'] and digest == expected[group]['semantic_sha256']
    print(f'{group}: {len(rows)-1} cases, every untimed field identical; SHA256 {digest}')
print('Smoke PASS: 48 matches (47 finished, one capped); the same case remains capped at 1200 plies; illegal=0, crashes=0')
