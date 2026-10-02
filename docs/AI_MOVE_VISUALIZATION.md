# AI move visualization — local beta.4 candidate

Logical behavior remains beta.3; the existing freeze also retains the older
Beta.2 public baseline: `775310a178da07b275af3ecf0817339cbbae19d2`; local baseline:
`d49c5d3`. The game, legality, AI, storage and power implementation/interface files
remain byte-identical under the 15-file freeze guard. Presentation lives in
`DgApp`, `src/ui/trails.c`, the common renderer and the existing native foreground.

## Final choice and atomic state

Search displays only THINKING. / THINKING.. / THINKING... through the existing
40-tick foreground cancellation hook. No candidate or principal variation moves
on screen. `dg_ai_choose()` returns its final move, ending RNG and stats before
`dg_find_move()` obtains the same deterministic shortest representative path
from the actual pre-board. `dg_commit()` validates and commits that move once;
ending RNG is assigned once. The renderer uses a 73-byte pre-board snapshot with
the moving source removed, and a separate `animation_actor` for TURN text.
The real board, turn, undo snapshot, winner and RNG are already final.

Every STEP/JUMP segment lasts **15 RTC ticks = 117.1875ms nominally**. Common
`dg_app_animation_tick()` quantizes visual position at 3 ticks: source, 20%, 40%,
60%, 80%, then the destination/next hop. There are four intermediate positions,
five advances per hop, six positions including both endpoints. The renderer
rounds interpolation between the real transformed centers; chains follow every
chosen path node. Elapsed time has one origin for the entire move, including
midnight wrapping, so delayed drawing skips frames without accumulating hop drift.

The existing shared foreground wake source is 20ms, with 64Hz RTC fallback.
Its ISR still sets only a flag. It allocates no per-search/per-move timer and
changes no RTC power threshold, search budget, RNG or physical-input activity.
Actual LCD frame cadence, drawing cost and hop duration remain
**HARDWARE TEST REQUIRED**; host timing is never calculator timing.

MENU/OFF/EXIT immediately finish visual replay and retain its completed trail.
MENU/OFF checkpoint the final committed state before the unchanged storage-close,
timer-pause, brightness-restore and OS handoff sequence. Busy EXIT checkpoints
directly to SETUP, bypassing normal deselect/zoom steps. Gameplay keys are ignored
while busy; busy softkeys are blank. Search cancellation still commits nothing.
A winning AI move marks the archive inactive and checkpoints the existing v1
**24-byte tombstone before replay**, then shows RESULT after replay or a system
skip. The final winning board stays in RAM; v1 does not serialize an inactive
board. No frame changes logical state or performs a per-frame flash write.

## Transient trails

Two fixed `DgAiTrail` slots retain chronological earlier/later AI paths. 2P uses
one GREEN slot. 3P retains the actual preceding GREEN and YELLOW paths, regardless
of their randomized order; the second AI preserves the first trail while searching
and replaying. Each AI's old path is replaced, never accumulated into history.

| Event | Trail behavior |
|---|---|
| AI replay completes or is skipped | Retain final representative path for that AI |
| Cursor, select/deselect, zoom, Assist visual change | Keep both |
| THINKING update or canceled search | Keep both |
| Invalid YOU destination | Keep both |
| Legal YOU commit | Clear both immediately |
| NEW, confirmed RESTART, successful UNDO | Clear both |
| Cold/global/local RESUME, fresh launch | Start without trails |
| EXIT to SETUP and later RESUME | Resume clears trails; game bytes remain correct |

No trail, animation board, actor, clock, phase or lane enters `DgArchive`, v1 save
payload or undo. New visual fields occupy 231 bytes; the aligned SH controller
grows from 376 to 608 bytes. Snapshot is 73 bytes; two 77-byte trails use 154;
actor/phase/ticks use four. The already existing `DgPath` is reused for replay.
There is one native framebuffer and no per-move heap allocation.

`DgPath.node` has the existing 74-node capacity. Exhaustive traversal of all
73 source nodes in the unconditional jump topology finds a largest component of
20 landing nodes. BFS representative paths never repeat a landing, so at most
19 hops/20 nodes are needed; STEP uses two nodes. The engine's existing bounded
capacity is sufficient without truncation. A capacity violation rejects visual
preparation before commit rather than inventing a partial route.

## Trim, direction and overlap

Geometry comes from transformed source/destination centers. Integer Q8 Euclidean
normalization supplies tangent and normal; there are no direction-specific
coordinate tables. Trim is six pixels in overview and nine in zoom: hole fill
radius plus black outline plus one-pixel gap. Normal lane offset is applied
before endpoint trim. Lines use 2px in overview and 3px in zoom, with flat
trimmed caps and a direction-independent raster. Each actual hop has exactly
one filled head in TRAIL_YELLOW or TRAIL_GREEN, matching its line. Head depth is
4px/5px; base span is 5px/7px before rotation. The whole STEP head is centered
in the open gap; a JUMP head uses the open interval after the crossed hole.
Q8 positioning avoids early distance rounding that would touch a diagonal hole.

Shared keys are `(min(from,to), max(from,to))`, including reverse movement.
Earlier/later lanes use -3/+3 nominal pixels along the **low-id to high-id
canonical normal**. Overview ±2px and ±2.5px were tested but shared a pixel at
expanded diagonal arrow wings; ±3px retains every pixel of both colors. Zoom
heads stagger by ±1px along the canonical tangent so their wider wings remain
separate. This changes only drawing geometry, not the actual path or direction.
Different-edge crossings use ordinary
z-order. A path's landing holes and any crossed occupied hole are protected by
rendering holes/pieces after trails. The cyan YOU route preview is below AI trails so it cannot erase a retained
path; cyan endpoint markers, cursor, selection and HUD still follow the holes.
The trail painter clips to x4..391/y26..203; overview HUD boxes stay outside the
board, and zoom HUD is drawn last in its compact backing boxes.

## Reproducible evidence

`test_visuals` checks 3,924 directed normalized segment/lane/view cases, actual
arrow pixels against every hole, thick-line endpoint clearance, exact axial
2px/3px widths, 48 same/opposite dual-lane cases with no color overwrite, complete
renderer layering, interpolation, atomic state and trail lifetimes. App/native
tests cover committed replay system requests, search cancellation, winning
modal/tombstone order, midnight timing, APO, save faults and unchanged idle.
The existing renderer test checks exact YOU and animation-actor HUD glyphs.

The freeze guard preserves 720 EASY goldens, all 192 fixed choices/RNG/nodes/
depth/beam and all 192 chosen representative paths recorded before edits.
[Smoke fingerprints](ai/ui-beta3-regression.json) and untimed fixtures retain
48 games: 47 finish, one remains capped at 400 and again at 1200 plies exactly
as before. Illegal moves/crashes are zero; no AI-strength or draw-rule change.

[Animation sheet](screenshots/animation-contact-sheet.png) contains all 26 frames
of an actual chosen five-hop path `66:47:45:43:27:41`, at ticks 0..75. The logical
move count stays one throughout; THINKING is absent. [Frame metadata](screenshots/animation-frames.csv)
records path/actor/phase/logical state. [Trail views](screenshots/ai-trails-sheet.png),
[all shared directions](screenshots/shared-lanes-sheet.png) and
[enlarged lane pixels](screenshots/shared-lane-details.png) come from the common
renderer. Long tactical boards are explicitly synthetic engine-valid fixtures;
both AI searches and commits are real. Shared-edge cases inject geometric paths
and are not claims that AI chose those illustrative routes.

[Palette audit](UI_COLOR_AUDIT.md), [validation](BETA_VALIDATION.md),
[memory](MEMORY.md) and [pending hardware checks](HARDWARE_RETEST.md) record
scope and limits. Physical animation speed, trail contrast and input response
remain HARDWARE TEST REQUIRED.

USB cancellation and closed-file handoff are documented in [USB audit](USB_LIFECYCLE_AUDIT.md). [Icon/trail comparisons](ICON_TRAIL_POLISH.md) cover the current local presentation candidate. No new binary is published before USB hardware validation.
