#!/usr/bin/env python3
"""Uniform-scale checks and historical previews; OS labels are explicit mocks."""
import argparse
import json
from pathlib import Path
from PIL import Image, ImageChops, ImageDraw
from generate_assets import ROOT, ICON_SCALE, ICON_TOP, icon, icon_artwork


def bounds(im):
    return ImageChops.difference(im, Image.new("RGB", im.size, "white")).getbbox()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true")
    parser.add_argument("--references", nargs=2, type=Path,
                        help="Optional read-only DIFF EQ and SOKOBAN assets directories")
    args = parser.parse_args()
    pairs = []
    records = []
    for selected, filename in [(False, "icon-uns.png"), (True, "icon-sel.png")]:
        old, new = icon_artwork(selected), icon(selected)
        previous = Image.new("RGB", old.size, "white")
        previous.paste(old, (0, -2))
        assert old.size == new.size == (92, 64)
        assert bounds(old) == (3, 2, 89, 52)
        assert bounds(previous) == (3, 0, 89, 50)
        assert bounds(new) == (6, 1, 86, 47)
        assert 0.90 <= ICON_SCALE <= 0.94 and ICON_TOP >= 1
        assert Image.open(ROOT / "assets" / filename).convert("RGB").tobytes() == new.tobytes()
        colors = lambda im: {im.getpixel((x, y)) for y in range(im.height) for x in range(im.width)}
        assert colors(new) <= colors(old)  # Nearest sampling adds no new colors.
        records.append(dict(variant=filename, beta3_bounds=bounds(old), previous_local_bounds=bounds(previous),
                            new_bounds=bounds(new), uniform_scale=ICON_SCALE, top_margin=ICON_TOP,
                            beta3_bottom_margin=12, previous_bottom_margin=14, new_bottom_margin=17,
                            physical_label_validation="HARDWARE TEST REQUIRED"))
        pairs.append((filename, old, previous, new))
    print("Icon PASS: normal/selected uniform 92% scale, 1px top and 17px bottom margins; hardware label check pending")
    if args.check:
        return
    out = ROOT / "docs/screenshots"
    out.mkdir(exist_ok=True)
    for scale in [1, 8]:
        w, h = 92 * scale, 64 * scale
        sheet = Image.new("RGB", (3*w+48, 2*(h+24)+8), "#d9dce0")
        draw = ImageDraw.Draw(sheet)
        for row, (name, old, previous, new) in enumerate(pairs):
            for col, (label, im) in enumerate([("beta.3", old), ("local", previous), ("beta.4 / 92%", new)]):
                x, y = 8+col*(w+16), 4+row*(h+24)
                draw.text((x, y), label+" "+("SEL" if row else "NORMAL"), fill="black")
                sheet.paste(im.resize((w, h), Image.Resampling.NEAREST), (x, y+18))
        sheet.save(out / f"icon-before-after-{scale}x.png")
    mock = Image.new("RGB", (672, 204), "#d9dce0")
    draw = ImageDraw.Draw(mock)
    for row, (name, old, previous, new) in enumerate(pairs):
        for col, (label, im) in enumerate([("beta.3", old), ("previous local", previous), ("beta.4", new)]):
            x, y = 10+col*224, 4+row*100
            draw.text((x, y), label+" / LABEL MOCK", fill="black")
            mock.paste(im, (x, y+16))
            draw.line((x, y+16+52, x+91, y+16+52), fill="#999999")
            draw.text((x+20, y+16+53), "DIAMOND", fill="black")
    mock.resize((1344, 408), Image.Resampling.NEAREST).save(out / "icon-label-safe-mock.png")
    (out / "icon-bounds.json").write_text(json.dumps(records, indent=2)+"\n")
    if args.references:
        # Reference-project images stay in ignored build/, not the public snapshot.
        sheet = Image.new("RGB", (960, 440), "#d9dce0")
        draw = ImageDraw.Draw(sheet)
        for col, (label, directory) in enumerate(zip(["DIFF EQ", "SOKOBAN", "DIAMOND"],
                                                     [*args.references, ROOT / "assets"])):
            for row, name in enumerate(["icon-uns.png", "icon-sel.png"]):
                im = Image.open(directory / name).convert("RGB")
                x, y = col*320+12, row*220+4
                draw.text((x, y), label+" / "+name, fill="black")
                sheet.paste(im.resize((276,192), Image.Resampling.NEAREST), (x,y+16))
        target = ROOT / "build/beta4-baseline/icon-reference-comparison.png"
        target.parent.mkdir(parents=True, exist_ok=True)
        sheet.save(target)


if __name__ == "__main__":
    main()
