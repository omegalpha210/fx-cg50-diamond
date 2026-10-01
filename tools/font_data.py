#!/usr/bin/env python3
"""Extract the pinned gint atlas as used by SOKOBAN and DIFF EQ (MIT helpers)."""
import argparse
import hashlib
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
atlas = ROOT / 'assets/font/font8x9.png'
assert hashlib.sha256(atlas.read_bytes()).hexdigest() == '0e7f56f30e021e16053360b972adc0b020c7d8a2d545fbd883bc8e7d870413c8'
im = Image.open(atlas).convert('RGB')
glyphs, widths = [], []
for i in range(95):
    col, row = i % (im.width // 10), i // (im.width // 10)
    g = im.crop((col * 10 + 1, row * 13 + 1, col * 10 + 9, row * 13 + 12))
    left, right = 0, 8

    def blank(x):
        return all(g.getpixel((x, y)) == (255, 255, 255) for y in range(11))

    while left + 1 < right and blank(left):
        left += 1
    while right - 1 > left and blank(right - 1):
        right -= 1
    widths.append(right - left)
    glyphs.append([sum((g.getpixel((x + left, y)) == (0, 0, 0)) << x
                       for x in range(right - left)) for y in range(11)])
out = '/* Generated from gint font8x9; see docs/LICENSE_AUDIT.md and THIRD_PARTY_NOTICES.md. */\n'
out += 'static const unsigned char glyph_width[95]={' + ','.join(map(str, widths)) + '};\n'
out += 'static const unsigned char glyph_rows[95][11]={\n' + ',\n'.join('{'+','.join(map(str, r))+'}' for r in glyphs) + '\n};\n'
destination = ROOT / 'src/ui/font_data.h'
if args.check:
    assert destination.read_text() == out, 'font table differs from pinned reference atlas'
    print('Font PASS: pinned gint atlas, exact SOKOBAN 95-glyph table, 1140 bytes')
else:
    destination.write_text(out)
