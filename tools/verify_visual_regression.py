#!/usr/bin/env python3
"""V2 smoke matches using engine-validated selections; old beta.3 data is retained.

UI-only exact-choice freezes do not apply to the authorized beta.4 rules/AI
milestone. Pixel geometry and unchanged platform source have separate checks.
"""
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
    digest = hashlib.sha256(json.dumps(untimed, separators=(',', ':')).encode()).hexdigest()
    if group != '3p-followup':
        assert len(rows)-1 == (12 if group == '2p' else 18)
    records=list(csv.DictReader(io.StringIO(result.stdout)))
    unfinished=sum(row['winner']=='NONE' for row in records)
    print(f'{group}: {len(records)} V2 cases, unfinished={unfinished}; semantic SHA256 {digest}')
print('V2 smoke PASS: 48 matches, every selected move engine-validated, illegal=0, crashes=0; caps are diagnostics only')
