#!/usr/bin/env python3
"""Capture common-renderer pixels and compose public-safe own-image evidence.

Before-fill PNGs are baseline renderer outputs, not private reference images.
A public checkout regenerates current images without reference projects.
"""
import csv
import json
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
    "setup-3p", "rules", "2p-overview", "3p-overview",
    "piece-readability", "piece-readability-zoom", "piece-selection",
    "assist-on", "assist-off", "zoom", "zoom-assist-off", "multi-jump",
    "restart", "undo-ready", "ai-thinking", "ai-animation", "warning",
    "result", "result-normal", "result-green-ai", "result-yellow-ai",
    "result-board", "player-resume", "move-with-undo", "rules-controls",
    "player-resume-2p", "player-resume-3p", "player-resume-saved-3p-tile-2p",
    "player-resume-saved-2p-tile-3p", "setup-2p-no-resume", "setup-3p-no-resume",
    "setup-2p-resume", "setup-3p-resume", "setup-2p-mismatch", "setup-3p-mismatch",
    "setup-2p-first-human", "setup-2p-first-ai", "setup-3p-human-1st",
    "setup-3p-human-2nd", "setup-3p-human-3rd", "setup-2p-assist-off",
    "setup-2p-assist-on", "setup-3p-assist-off", "setup-3p-assist-on",
    "hud-human-2p-easy", "hud-yellow-3p-normal", "hud-green-3p-hard",
    "thinking-1", "thinking-2", "thinking-3", "thinking-zoom-1", "thinking-zoom-2",
    "thinking-zoom-3", "zoom-unselected", "selected-overview", "selected-zoom",
    "warning-long", "warning-zoom",
]
VISUAL_SCENES = ["trails-you", "green-long-trail", "yellow-long-trail",
                 "trails-both", "trails-thinking", "trails-zoom", "trails-assist",
                 "trails-selected", "trails-assist-zoom", "yellow-long-trail-zoom", "trails-2p"]
SHARED_SCENES = [f"shared-{kind}-d{direction}-{view}"
                 for view in ["overview", "zoom"] for direction in range(6)
                 for kind in ["same", "opposite"]]
SCENES += VISUAL_SCENES + SHARED_SCENES
subprocess.run([str(ROOT / "build/host/capture"), "docs/screenshots"], cwd=ROOT, check=True)
frames = list(CAPTURES.glob("*.ppm"))
for obsolete in ["settings.png", "level-faces.png"]:
    (CAPTURES / obsolete).unlink(missing_ok=True)
for path in frames:
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
sheet([[("NO RESUME / 2P", CAPTURES / "setup-2p-no-resume.png"),
        ("MATCHING RESUME / 2P", CAPTURES / "setup-2p-resume.png")],
       [("NO RESUME / 3P", CAPTURES / "setup-3p-no-resume.png"),
        ("MATCHING RESUME / 3P", CAPTURES / "setup-3p-resume.png")]], CAPTURES / "setup-rows-sheet.png")
sheet([[(f"THINKING / phase {phase}", CAPTURES / f"thinking-{phase}.png")
        for phase in [1, 2, 3]]], CAPTURES / "thinking-phases.png")
with Image.open(CAPTURES / "player.png") as frame:
    frame.crop((45, 86, 154, 124)).resize((654, 228), Image.Resampling.NEAREST).save(
        CAPTURES / "player-icons.png")
POLISH_BEFORE = CAPTURES / "beta1-before"
if POLISH_BEFORE.exists() and (CAPTURES / "beta2-before").exists():
    pairs = [("PLAYER", "player", "player"), ("SETUP", "setup-3p", "setup-3p-resume"),
             ("HUD", "3p-overview", "hud-yellow-3p-normal"),
             ("THINKING", "ai-thinking", "thinking-3"), ("WARNING", "warning", "warning-long")]
    sheet([[(f"{label} / beta.1", POLISH_BEFORE / f"{old}.png"),
            (f"{label} / beta.2", CAPTURES / "beta2-before" / f"{new}.png")] for label, old, new in pairs],
          CAPTURES / "ui-polish-before-after.png")

with (CAPTURES / "animation-frames.csv").open(newline="") as stream:
    animation = list(csv.DictReader(stream))
assert animation and animation[0]["rtc_ticks"] == "0" and animation[-1]["animation"] == "0"
assert len({row["path"] for row in animation}) == 1
assert len({row["committed_turns"] for row in animation}) == 1
animation_cells = [(f"{row['rtc_ticks']} RTC ticks / " +
                    ("FINAL + TRAIL" if row["animation"] == "0" else
                     f"hop {int(row['segment']) + 1} / phase {row['phase']}"),
                    CAPTURES / f"{row['scene']}.png") for row in animation]
sheet([animation_cells[i:i + 3] for i in range(0, len(animation_cells), 3)],
      CAPTURES / "animation-contact-sheet.png")
sheet([[(name, CAPTURES / f"{name}.png") for name in VISUAL_SCENES[i:i + 3]]
       for i in range(0, len(VISUAL_SCENES), 3)], CAPTURES / "ai-trails-sheet.png")
sheet([[(name, CAPTURES / f"{name}.png") for name in SHARED_SCENES[i:i + 2]]
       for i in range(0, len(SHARED_SCENES), 2)], CAPTURES / "shared-lanes-sheet.png")
sheet([[ ("YELLOW fill / text / NORMAL / trail", CAPTURES / "yellow-long-trail.png"),
         ("TURN YELLOW / progress / NORMAL", CAPTURES / "hud-yellow-3p-normal.png")],
       [("NORMAL option / shared semantic gold", CAPTURES / "setup-3p-normal.png"),
        ("YOU / GREEN / YELLOW trails", CAPTURES / "trails-you.png")],
       [("Saved NORMAL / same gold", CAPTURES / "setup-3p-resume.png"),
        ("Result NORMAL / same gold", CAPTURES / "result-normal.png")]],
      CAPTURES / "yellow-palette-sheet.png")
# Enlargements are nearest-neighbor crops of actual shared-lane renderer pixels.
image = Image.new("RGB", (4 * 240, 6 * 226), "#d9dce0")
draw = ImageDraw.Draw(image)
for direction in range(6):
    for column, (view, kind) in enumerate([(v, k) for v in ["overview", "zoom"]
                                         for k in ["same", "opposite"]]):
        name = f"shared-{kind}-d{direction}-{view}"
        with Image.open(CAPTURES / f"{name}.png") as frame:
            crop = frame.crop((174, 90, 222, 138)).resize((192, 192), Image.Resampling.NEAREST)
        x, y = column * 240, direction * 226
        caption(draw, (x + 4, y + 4), f"d{direction} {kind} {view}")
        image.paste(crop, (x + 20, y + 22))
image.save(CAPTURES / "shared-lane-details.png")
(CAPTURES / "capture-manifest.json").write_text(json.dumps({
    "tag": "v0.1.0-beta.3", "current_renderer_frames": len(frames),
    "animation_frames": len(animation), "representative_path": animation[0]["path"],
    "hop_rtc_ticks": 15, "frame_rtc_ticks": 3,
    "provenance": "common dg_render plus actual dg_app_cpu/dg_app_animation_tick; shared-lane fixtures explicitly synthetic",
    "hardware_timing": "HARDWARE TEST REQUIRED"
}, indent=2) + "\n")

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

print(f"{len(frames)} current actual-renderer frames; public-safe setup, phase, profile, piece and overview sheets")
