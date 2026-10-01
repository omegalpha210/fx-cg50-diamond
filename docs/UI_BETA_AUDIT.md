# Beta UI change audit

This pass implements the requested three-level presentation and calculator
readability changes on the common host/native renderer. The baseline drawing
implementation is local commit `1f26635`. All screenshots and comparison images
here are generated from DIAMOND source; reference-project screenshots and
article/product artwork are not needed for regeneration or publication.

## Difficulty controls and wording

LEVEL contains three 122×46 buttons on one row, in **EASY, NORMAL, HARD** order.
Each always includes its name and a colored face: green smile, yellow neutral,
red frown with simple eyebrows. The faces have a black circular outline, two
eyes and a mouth. The selected button also has a blue two-pixel border and blue
backing; color is not the only selection cue.

Stored difficulty IDs remain EASY `0`, HARD `1`, NORMAL `2`. `src/ui/app.c`
changes only the LEVEL selector mapping: left/right walks the explicit display
order and clamps at both ends. FIRST/HUMAN selectors, EXE start behavior,
navigation, restart, undo, zoom, turn order and checkpoint semantics retain their
existing implementation. The renderer displays NORMAL by ID in setup, the HUD
and result statistics; an existing HARD remains HARD.

PLAYER and red `(YOU ARE RED)` share a measured font baseline. Tile circles
retain their compact layout; RED is a solid human circle, while GREEN and YELLOW
have centered black `AI` glyphs. The 2P FIRST labels are HUMAN / AI. Turn and
result labels use GREEN AI / YELLOW AI and AI WINS. Existing internal CPU warning
strings are mapped to AI text by the renderer, preserving controller behavior
and internal function names. Restart text says “Same setup and AI order.”

## Board and Assist

Occupied holes have a pure black outline and a completely color-filled interior.
The former inner white ring and white mechanical marks are removed from board
pieces. Empty holes retain white interiors. Overview uses outer radius 5 and
interior radius 4; zoom uses outer radius 8 and interior radius 7. Their existing
centers, circle bounds and board coordinate transform are unchanged.

Legal empty Assist destinations use the same thin black outline with a fully
cyan interior. The outer cyan target ring and inner crosshair are removed in
both views. Engine-generated endpoints and representative path previews are
unchanged. Assist OFF renders white empty interiors and retains illegal-move
warnings. The blue cursor and yellow/dark selected-piece rings keep their
existing geometry and colors; a cyan focused destination remains fully filled
inside its blue cursor.

Every displayed RULES action now uses pure black background and white text,
without an F-number. Other semantic softkey colors retain their existing values.

## Actual renderer evidence

`tools/capture.c` produces 37 current 396×224 frames. `tools/ui_captures.py`
converts those PPM pixels to PNG and assembles only the project's own images.
Comparison captions use the same generated gint glyph atlas as the app, under
the permission recorded in [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md);
no separate caption font is bundled or embedded.
The driver checks piece-count-valid fixtures and legal representative routes.
It exports crop centers through `dg_screen_position()`, so Python does not
reconstruct board geometry or draw replacement UI pixels.

- [All three level faces](screenshots/level-faces.png) is a nearest-neighbor
  enlarged crop from the actual NORMAL-selected 2P setup frame.
- [Every level selected in both modes](screenshots/level-selection-sheet.png)
  shows six complete setup frames.
- [Piece before/after](screenshots/piece-before-after.png) compares RED, GREEN,
  YELLOW, white empty holes and cyan destinations in overview and zoom.
- [Full-frame before/after](screenshots/beta-before-after.png) compares PLAYER,
  setup, overview, Assist, zoom and result.
- [Current contact sheet](screenshots/contact-sheet.png) includes all required
  screen types; [capture index](screenshots/README.md) links the original frames.

The frozen before-fill samples use baseline `1f26635` drawing functions with
the same legal ten-piece fixture driver as the new renderer. They are retained
under `docs/screenshots/before-fill`; regenerating current images needs neither
private references nor a historical checkout. Enlargements use integer nearest
neighbor resampling, with original-size PNGs kept separately.

Exact fill pixels measured inside the unchanged interior circle are recorded in
[piece-fill-metrics.csv](screenshots/piece-fill-metrics.csv):

| Interior | Overview before → after | Zoom before → after |
| --- | ---: | ---: |
| RED / GREEN / YELLOW | 11 → 49 each | 87 → 149 each |
| Empty white | 49 → 49 | 149 → 149 |
| Cyan destination | 5 → 49 | 5 → 149 |

These counts measure renderer pixels, not LCD perception or calculator timing.

## Verification

The strict host and UBSan UI controller tests pass with explicit stable IDs and clamped
EASY↔NORMAL↔HARD transitions. The expanded renderer geometry test passes 227
immutable frames and 1,180,925 bounded rectangle submissions. Pixel contracts
check complete softkey strips, all three selected borders, every face pixel,
black AI glyphs inside each colored tile circle, same-baseline red annotation,
every board-disc interior/outline pixel in both views, absence of cyan outer
rings/crosshairs, cyan/blue cursor contrast, Assist OFF, proportional fonts,
HUD bounds, all navigation nodes, notices and result/restart modals.
The same expanded renderer test also passes under UBSan. Each of the five UI
translation units independently passes strict SH compilation with zero warnings.

Final complete host/UBSan/SH/package results belong to the release validation
report. Actual color distinction, 1-pixel outline readability, face expressions,
physical controls and AI latency remain **HARDWARE TEST REQUIRED**, as listed
in [HARDWARE_RETEST.md](HARDWARE_RETEST.md). The supported control conventions
are recorded in [UI_CONVENTIONS.md](UI_CONVENTIONS.md).
