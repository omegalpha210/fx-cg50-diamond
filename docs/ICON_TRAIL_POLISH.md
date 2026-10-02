# Icon and trail polish — beta.4

Current icon bounds are `(6,1,86,47)` for normal and selected variants, with
uniform 92% affine scaling, one white top row and 17 clear bottom rows. The
previous local translation had bounds `(3,0,89,50)` and 14 bottom rows;
beta.3 had `(3,2,89,52)` and 12. All three appear in the native/8× comparison.
The source artwork and colors are preserved through nearest-neighbor sampling.
Physical OS-label overlap remains HARDWARE TEST REQUIRED.

See [beta.4 evidence](BETA4_AUDIT.md), [icon comparison](screenshots/icon-before-after-1x.png),
[8× view](screenshots/icon-before-after-8x.png), [label mock](screenshots/icon-label-safe-mock.png)
and [measured bounds](screenshots/icon-bounds.json).

## Trail geometry and color

| Property | Beta.3 | Candidate |
| --- | --- | --- |
| YELLOW trail/arrow | text gold `0xac00` | independent bright `0xf5c0` |
| GREEN trail/arrow | dark text green `0x0508` | independent light green `0x5644` |
| Cyan Assist | `0x05da` | unchanged |
| Overview stroke | 1px | 2px |
| Zoom stroke | 1px | 3px |
| Arrow | 2/3px-deep 1px chevron | filled 4/5px-deep head, 5/7px base span |
| Shared lane offset | ±1px | ±3px canonical normal, both views |
| Endpoint trim | 6/9px | unchanged, verified with thick strokes |

Each actual STEP or JUMP has exactly one head in its line color. Q8 geometry
centers the full head in the gap; JUMP uses the gap after the crossed hole.
Zoom heads stagger ±1px along the canonical tangent. Overview ±2px and ±2.5px
were rejected because diagonal arrow wings shared a pixel. The final ±3px
lanes retain every pixel of both colors in all 48 same/opposite direction cases.
All 3,924 directed lane/view cases keep arrow pixels outside every hole and
stroke endpoints outside their source/destination holes. Jump shafts can cross
the jumped-over hole; the existing later hole/piece layer covers that crossing.

Layer order remains background/camps, edges, cyan YOU preview, AI strokes and
arrows, holes/pieces, Assist markers, moving piece, selection/cursor, HUD. No
trail covers a piece or HUD. Text and piece palettes are unchanged.

[Before/after sheet](screenshots/trail-before-after.png),
[16 required overview/zoom views](screenshots/trail-required-views.png),
[all shared directions](screenshots/shared-lane-details.png),
[overview contrast fixture](screenshots/trail-contrast-overview.png), and
[zoom contrast fixture](screenshots/trail-contrast-zoom.png) come from the common
C renderer/primitives. Long moves use actual AI choices on engine-valid tactical
fixtures. Single-step/shared-edge and background swatches are explicitly
geometric illustrations, not claims that an AI chose those synthetic routes.
The swatches include neutral, yellow/green/red camps, white holes, colored pieces
and cyan Assist. [Color-only ratios](screenshots/trail-contrast.json) report the
lower luminance contrast of brighter colors honestly; thicker strokes and larger
heads add visible area. Only an actual LCD test can settle physical readability.

## Reproduction and preserved presentation

`python3 tools/icon_previews.py` regenerates the public original-artwork
comparisons. `--references` optionally reads the two external reference asset
directories and writes only an ignored private comparison. `tools/ui_captures.py`
regenerates 145 current common-renderer frames; `tools/polish_previews.py` records
trail geometry/color comparisons. The 26-frame five-hop presentation fixture
remains a historical V1 synthetic route. New rules and AI are covered separately.

The actual trail source, tokens, native loop and RTC constants are preserved.
Endgame workspace and save revision changed intentionally in beta.4; they are
not represented as an unchanged whole-engine UI-only milestone. USB remains
experimental with hardware tests pending, as permitted for this beta release.
