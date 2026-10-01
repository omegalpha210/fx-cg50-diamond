#!/usr/bin/env python3
"""Capture common-renderer pixels and compose public-safe own-image evidence.

Before-fill PNGs are baseline renderer outputs, not private reference images.
A public checkout regenerates current images without reference projects.
"""
import csv
from pathlib import Path
import re
import subprocess
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
CAPTURES = ROOT / "docs/screenshots"
BEFORE = CAPTURES / "before-fill"


def caption_font():
    """Reuse the renderer's gint atlas without introducing another font asset."""
    source = (ROOT / "src/ui/font_data.h").read_text()
    widths = re.search(r"glyph_width\[95\]\s*=\s*\{([^}]+)\}", source)
    rows = re.search(r"glyph_rows\[95\]\[11\]\s*=\s*\{(.*?)\n\};", source, re.S)
    if not widths or not rows:
        raise ValueError("Renderer caption font table missing")
    glyph_width = [int(value) for value in widths.group(1).split(",")]
    glyph_rows = [[int(value) for value in row.split(",")]
                  for row in re.findall(r"\{([^}]+)\}", rows.group(1))]
    if len(glyph_width) != 95 or len(glyph_rows) != 95 or any(
            len(row) != 11 for row in glyph_rows):
        raise ValueError("Renderer caption font dimensions changed")
    return glyph_width, glyph_rows


GLYPH_WIDTH, GLYPH_ROWS = caption_font()


def caption(draw, position, label):
    x, y = position
    for character in label:
        code = ord(character)
        glyph = (code if 32 <= code <= 126 else ord("?")) - 32
        for row, bits in enumerate(GLYPH_ROWS[glyph]):
            for column in range(GLYPH_WIDTH[glyph]):
                if bits & (1 << column):
                    draw.point((x + column, y + row), fill="black")
        x += GLYPH_WIDTH[glyph] + 1


SCENES = [
    "player", "player-2p", "setup-2p-easy", "setup-2p-normal", "setup-2p-hard",
    "setup-3p", "settings", "rules", "2p-overview", "3p-overview",
    "piece-readability", "piece-readability-zoom", "piece-selection",
    "assist-on", "assist-off", "zoom", "zoom-assist-off", "multi-jump",
    "restart", "undo-ready", "ai-thinking", "ai-animation", "warning",
    "result", "result-normal", "result-green-ai", "result-yellow-ai",
    "result-board", "player-resume", "move-with-undo", "rules-controls",
]
subprocess.run([str(ROOT / "build/host/capture"), "docs/screenshots"], cwd=ROOT, check=True)
for path in CAPTURES.glob("*.ppm"):
    with Image.open(path) as original:
        original.save(path.with_suffix(".png"))
    path.unlink()


def sheet(rows, destination):
    columns = max(len(row) for row in rows)
    image = Image.new("RGB", (columns * 412 + 8, len(rows) * 252 + 8), "#d9dce0")
    draw = ImageDraw.Draw(image)
    for i, row in enumerate(rows):
        for j, (label, path) in enumerate(row):
            x, y = 8 + j * 412, 6 + i * 252
            caption(draw, (x, y), label)
            with Image.open(path) as original:
                image.paste(original.convert("RGB"), (x, y + 20))
    image.save(destination)


sheet([[(name, CAPTURES / f"{name}.png") for name in SCENES[i:i + 3]]
       for i in range(0, len(SCENES), 3)], CAPTURES / "contact-sheet.png")
sheet([[(f"{players} / {level.upper()}", CAPTURES / f"setup-{players}-{level}.png")
        for level in ["easy", "normal", "hard"]] for players in ["2p", "3p"]],
      CAPTURES / "level-selection-sheet.png")
# Integer nearest-neighbor enlargement preserves actual face/text pixels.
with Image.open(CAPTURES / "setup-2p-normal.png") as frame:
    frame.crop((3, 28, 393, 108)).resize((1170, 240), Image.Resampling.NEAREST).save(
        CAPTURES / "level-faces.png")

PALETTE = {
    "red": (28 * 255 // 31, 3 * 255 // 31, 3 * 255 // 31),
    "green": (0, 20 * 255 // 31, 8 * 255 // 31),
    "yellow": (255, 24 * 255 // 31, 0),
    "empty": (255, 255, 255),
    "cyan": (0, 23 * 255 // 31, 26 * 255 // 31),
}
KIND = ["red", "green", "yellow", "empty", "cyan"]


def samples(directory):
    with (directory / "piece-samples.csv").open(newline="") as stream:
        return {(row["kind"], int(row["zoom"])): row for row in csv.DictReader(stream)}


def crop_sample(directory, sample):
    x, y, radius = (int(sample[key]) for key in ["x", "y", "radius"])
    half = radius + 5
    with Image.open(directory / f"{sample['scene']}.png") as frame:
        return frame.crop((x - half, y - half, x + half + 1, y + half + 1)).convert("RGB")


after = samples(CAPTURES)
if (BEFORE / "piece-samples.csv").exists():
    before = samples(BEFORE)
    image = Image.new("RGB", (5 * 216, 4 * 248), "#d9dce0")
    draw = ImageDraw.Draw(image)
    metrics = []
    for row, (label, directory, metadata, zoom) in enumerate([
        ("BEFORE / overview", BEFORE, before, 0),
        ("AFTER / overview", CAPTURES, after, 0),
        ("BEFORE / zoom", BEFORE, before, 1),
        ("AFTER / zoom", CAPTURES, after, 1),
    ]):
        for col, kind in enumerate(KIND):
            sample = metadata[kind, zoom]
            original = crop_sample(directory, sample)
            expanded = original.resize((original.width * 8, original.height * 8),
                                       Image.Resampling.NEAREST)
            x, y = col * 216, row * 248
            caption(draw, (x + 6, y + 5), f"{label}: {kind.upper()}")
            image.paste(expanded, (x + (216 - expanded.width) // 2, y + 26))
            radius = int(sample["radius"])
            center = original.width // 2
            interior = sum(
                original.getpixel((center + dx, center + dy)) == PALETTE[kind]
                for dy in range(-radius, radius + 1)
                for dx in range(-radius, radius + 1)
                if dx * dx + dy * dy <= radius * radius
            )
            metrics.append([label.split(" / ")[0].lower(), zoom, kind, radius, interior])
    image.save(CAPTURES / "piece-before-after.png")
    with (CAPTURES / "piece-fill-metrics.csv").open("w", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["version", "zoom", "kind", "interior_radius", "exact_fill_pixels"])
        writer.writerows(metrics)
    comparison = ["player", "setup-3p", "3p-overview", "assist-on", "zoom", "result"]
    sheet([[(f"{name} / before", BEFORE / f"{name}.png"),
            (f"{name} / after", CAPTURES / f"{name}.png")] for name in comparison],
          CAPTURES / "beta-before-after.png")
else:
    print("Frozen own before-fill samples absent; current renderer captures are complete.")

print("37 current actual-renderer frames; public-safe level, piece and overview sheets")
