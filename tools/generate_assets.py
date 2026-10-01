#!/usr/bin/env python3
"""Draw original package icons and convert the actual C renderer's PPM captures.

No downloaded image, external font, or template icon is used.
"""
from pathlib import Path
import argparse
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]

def icon(selected):
    image = Image.new("RGB", (92, 64), (255, 255, 255))
    draw = ImageDraw.Draw(image)
    paper = (226, 241, 250) if selected else (246, 249, 251)
    draw.rounded_rectangle((3, 2, 88, 51), radius=7, fill=paper,
                           outline=(26, 57, 81), width=2 if selected else 1)
    # A schematic six-point star authored from exact mathematical vertices.
    star = [(46, 5), (54, 18), (70, 18), (62, 31), (70, 44), (54, 44),
            (46, 51), (38, 44), (22, 44), (30, 31), (22, 18), (38, 18)]
    draw.polygon(star, fill=(255, 255, 255), outline=(120, 143, 157))
    points = []
    for r in range(-6, 7):
        for q in range(-6, 7):
            cube = (q, r, -q-r)
            if min(cube) >= -3 or max(cube) <= 3:
                points.append((46+(2*q+r)*2.6, 28+r*3.7))
    for x, y in points:
        draw.ellipse((x-.7, y-.7, x+.7, y+.7), fill=(87, 111, 126))
    for x, y, color in [(46, 41, (228, 39, 43)), (34, 20, (249, 207, 20)),
                         (58, 20, (16, 164, 89))]:
        draw.ellipse((x-5, y-3, x+5, y+5), fill=(37, 54, 67))
        draw.polygon([(x-4, y+3), (x-2, y-6), (x+2, y-6), (x+4, y+3)], fill=color)
        draw.ellipse((x-3, y-8, x+3, y-2), fill=color, outline=(37, 54, 67))
        draw.line((x-1, y-6, x-1, y-4), fill=(255, 255, 255), width=1)
    # Rows 53..63 are intentionally clear for the OS label area.
    return image

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--captures", type=Path)
    args = parser.parse_args()
    (ROOT / "assets").mkdir(exist_ok=True)
    for selected, name in [(False, "icon-uns.png"), (True, "icon-sel.png")]:
        target = ROOT / "assets" / name
        icon(selected).save(target)
        print(target.relative_to(ROOT))
    if args.captures:
        for source in sorted(args.captures.glob("*.ppm")):
            image = Image.open(source)
            assert image.size == (396, 224)
            image.save(source.with_suffix(".png"))
            source.unlink()
        print(f"PNG captures: {args.captures}")

if __name__ == "__main__":
    main()
