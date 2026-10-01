#!/usr/bin/env python3
"""Preserve rules/input/storage while allowing the requested difficulty/UI audit."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import re

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / 'tests/fixtures'
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--executable', type=Path, required=True)
parser.add_argument('--records', action='store_true', help='retained compatibility; rules are always checked')
args = parser.parse_args()
manifest = json.loads((BASE / 'frozen-engine-sha256.json').read_text())
for name, expected in manifest.items():
    actual = hashlib.sha256((ROOT / name).read_bytes()).hexdigest()
    assert actual == expected, f'frozen file changed: {name}'
# Normalize exactly the requested difficulty validation and selector changes.
game = (ROOT / 'src/game/game.c').read_text()
game = game.replace('!dg_level_valid(game->level)', 'game->level>DG_HARD')
game = game.replace('!dg_level_valid(level)', 'level>DG_HARD')
assert hashlib.sha256(game.encode()).hexdigest() == 'c6efd95624e23af9b67ee0ead400d76529352aebfd9ab44335599fff00aa9cbf'
header = (ROOT / 'include/diamond.h').read_text()
header = header.replace('/* Persisted v1 IDs. Display order is EASY, NORMAL, HARD. */\nenum { DG_EASY=0, DG_HARD=1, DG_NORMAL=2 };\nstatic inline bool dg_level_valid(uint8_t level){return level<=DG_NORMAL;}', 'enum { DG_EASY, DG_HARD };')
assert hashlib.sha256(header.encode()).hexdigest() == '5c191e9b3c179c9548c5df2e8df6d335465a7b3ac64dd55c5957d0d363267398', 'unintended engine interface change'
app = (ROOT / 'src/ui/app.c').read_text()
app = re.sub(r'static uint8_t adjust_level\(uint8_t value,int direction\)\n\{.*?\n\}\n', '', app, flags=re.S)
app = app.replace('adjust_level(app->level,direction)', 'adjust(app->level,direction,DG_HARD)')
assert hashlib.sha256(app.encode()).hexdigest() == '44cc87e751c29be8a2547a21c25213949f7892caa2c265f486a80f6a07e49274', 'unintended input change'
golden = subprocess.check_output([str(args.executable.resolve())], text=True)
def preserved(text):
    return [line for line in text.splitlines() if ' HARD ' not in line]
assert preserved(golden) == preserved((BASE / 'engine-golden-legacy.txt').read_text()), 'topology/moves/EASY/undo/save golden changed'
new = (ROOT / 'src/ui/menus.c').read_text().split('lines[]={', 1)[1].split('};', 1)[0]
assert re.findall(r'"([^"\n]*)"', new) == json.loads((BASE / 'rules.json').read_text()), 'rule strings changed'
print(f'Freeze PASS: {len(manifest)} byte-identical files; game/input changes limited to NORMAL; topology, moves/paths, EASY choices/RNG, undo, v1 save and 21 rule strings preserved')
