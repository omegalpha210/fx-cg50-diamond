# UI polish beta.2 audit

Baseline: local `42e5a55e49c53dd2a4cf5a291ec55c076b2817b5`, public
`f7e111c9c2a12243275875473fbbc30ecb34d628`, release `v0.1.0-beta.1`.
Before any edits, the actual host tools recorded 720 EASY goldens, 192 fixed
choices with RNG/nodes/depth/beam and complete engine/undo/save golden output.
Thirteen engine/game/AI/storage/power files were hashed; both original engine/AI
unit sources are also frozen. The [fixtures](../tests/fixtures/README.md) carry
the public-safe untimed records. [Before captures](screenshots/beta1-before/README.md)
are own renderer pixels, not reference-project screenshots.

## Direct NUM GAME reference

Read-only NUM GAME revision `825b5561914a198629a0490fa229c956c3abdfc6` was read
from source before implementation. Its working tree was clean before inspection.

| Actual source/function | Applied convention |
|---|---|
| `src/ui/entry.c`: `ng_entry_count`, `ng_entry_action`, `ng_entry_row` | Same-game RESUME conditional row, shared count/action mapping |
| `src/ui/render.c`: `entry` | Numbered normal-font rows, default row focus, aligned number/label/value columns, compact five-row layout |
| `src/ui/render.c`: `difficulty_options`, `mode_options` | Measured option width, selected outline/underline, visible values and colored labels |
| `src/ui/render.c`: `soft` | Action-only dark/white OPEN; blank unused slots |
| `src/core/app.c`: `enter`, `ng_app_key` NG_ENTRY branch | Default RESUME else NEW; UP/DOWN row focus; LEFT/RIGHT options only; EXE/F6 resumes on RESUME and starts NEW otherwise |

DIAMOND keeps the requested F4 RULES placement and green/gold/red difficulty
palette. Gold exactly matches the reference RGB5(25,12,0). The reference rows
use x10/w376, number x20, label x43, options x146 with 228px content area.
These dimensions and normal font are used directly, with four/five-row pitch.
No face selector or secondary FIRST/HUMAN block remains.

## THINKING and geometry

Old: fixed static circular indicator plus THINKING..., backing (6,57,117,18),
text origin (22,60). Its backing extended to x123, crossing the hole/outline
bbox at x121 by 2px and the conservative cursor-inclusive bbox by 8px.
New: text only, phases THINKING. / THINKING.. / THINKING..., fixed backing
(6,57,75,15) and origin (8,59). Maximum measured string width is 71px.
Its right edge x81 leaves 34px before the conservative board bbox x115.

`cancel_search()` already polls bounded input and idle. After cancellation checks,
it reads elapsed RTC time and calls `dg_app_thinking_tick()`; a phase change uses
`dg_render_thinking()` and `dupdate()` to clear/redraw only the fixed box. Forty
128Hz RTC ticks are 312.5ms; midnight wrap uses the existing day convention.
No new timer is allocated. Cancellation takes priority over drawing. AI code,
profiles, node order, RNG, budgets and committed state are untouched. Visual
updates do not call physical-input activity and cannot keep idle alive.

All coordinates below are half-open; width includes measured advances, not
character counts. The font cell height is 11; compact panels add 2px per side.

| Overlay | Maximum glyph / backing bounds | Overview audit |
|---|---|---|
| TURN prefix + longest actor | Prefix x8/y6/w41; actor x54/y6/w71; dark prefix, actor color | Inside y0..24 header; board starts y27 |
| Mode/difficulty | Longest 83px, x156/y6; actual-width centered, entire difficulty color | Inside header |
| Counter | UINT32_MAX: 94px at x294/y6 | Inside header, separated from centered mode |
| ASSIST | OFF: 78px; backing (6,30,82,15); ON backing width74 | Right edge ≤88; ≥27px board gap |
| YOU progress | At most 69px; backing x317/w73 | Outside x282 board right edge |
| YELLOW progress | At most 94px; backing x292/w98 | ≥10px gap; 3P only |
| GREEN progress | At most 83px; backing x303/w87 | Outside board; row 2 in 2P, row 3 in 3P |
| Goal rows | y30,45,60; height15 each; full colored text, no dots | Measured widths, no oversized panel |
| THINKING | (6,57,75,15), maximum text 71px | 34px horizontal gap, every phase same box |
| Warning | x6/y84, width≤107, height≤54; wrapped 11px text + padding | Right edge≤113, ≥2px gap |

The overview board envelope includes hole outlines, selected rings and every
possible cursor tick: (115,27,167,175). Tests compute it from the real node
coordinates. All real paper backing rectangles are collected from the renderer
and checked against it. Zoom allows board-under-panel overlap but retains the
same measured maxima. Full-pixel phase masks prove absence of the old circle
and erase previous dots on a 3→1 update; all pixels outside that box remain equal.

## PLAYER and exact SETUP rows

The human circle has a white radius-4 head and clipped integer shoulder disk,
with no facial features. AI retains centered black glyphs. F1 is global RESUME
for any valid unfinished save, including saved 3P with a 2P tile selected and
vice versa. F2 is blank; SET and the ASSIST-only SETTINGS navigation are gone.

| Mode/save match | Numbered rows in order |
|---|---|
| 2P, no matching resume | 1 NEW GAME; 2 DIFFICULTY; 3 FIRST; 4 ASSIST |
| 2P, matching resume | 1 RESUME; 2 NEW GAME; 3 DIFFICULTY; 4 FIRST; 5 ASSIST |
| 3P, no matching resume | 1 NEW GAME; 2 DIFFICULTY; 3 HUMAN; 4 ASSIST |
| 3P, matching resume | 1 RESUME; 2 NEW GAME; 3 DIFFICULTY; 4 HUMAN; 5 ASSIST |

LEFT/RIGHT action rows are no-ops; options clamp. EXE/F6 OPEN never cycles an
option. Every non-RESUME row starts NEW with current settings through the
unchanged verified replacement transaction. RESUME restores game bytes exactly,
even after editing next-game difficulty/slot. ASSIST remains global, persisted
at the original field and checkpointed on setup EXIT/MENU/OFF or NEW. A changed
global Assist can apply to RESUME without changing saved game bytes. Three-player
NEW retains existing randomization of only the two remaining AI colors.

## EXIT precedence and tests

| Context | EXIT result |
|---|---|
| Selected + overview | Deselect; next EXIT checkpoints to SETUP |
| Selected + zoom | Deselect and remain zoom; next EXIT overview; third EXIT SETUP |
| No selection + zoom | Overview, stay in game |
| No selection + overview | Checkpoint → SETUP, unfinished RESUME retained |
| CPU search/preview busy, including zoom | Cancel pending move/RNG → checkpoint → SETUP |
| Restart confirmation | Cancel modal, preserve board/zoom/selection |
| RULES | Previous screen, preserve game context |
| Result | Close result to view frozen board; existing NEW behavior retained |

The native `cpu_turn()` explicitly routes a pending busy EXIT to setup after
`dg_app_cpu()` clears thinking, avoiding an accidental normal zoom-back step.
No CPU move or pending RNG is committed by cancellation. Save failures retain
RAM and block the final setup/lifecycle transition, with the existing retry notice.

## Verification

`ui_freeze` requires all 15 frozen file hashes and exact engine output plus all
192 choices/RNG/nodes/depth/beam; 720 EASY golden choices remain in unchanged
unit tests. Native clock-stepped tests run the real foreground search callback
for all six mode/level combinations, see all three visual phases, and compare
pending move/RNG/stats to an unanimated reference. Idle accumulates, no storage
writes or timer allocation occur, and midnight phase boundaries are tested.

`test_app` covers 4/5 rows, ASSIST last, mode mismatch, F1 global restoration,
row navigation/defaults, action no-ops, all clamps, every non-RESUME EXE/OPEN
start across levels/slots/Assist, exact local restore, global Assist cold load,
verified NEW failure, normal EXIT matrix and modal/busy priority. Original
save/undo/restart/completion cases remain. Renderer tests cover every option
and focus in both row layouts, exact softkey strips, profile/AI pixels, all
HUD colors/fractions, centered glyph widths, actual panel bounds and partial
phase clearing. [Acceptance](ACCEPTANCE.md) and [release validation](BETA_VALIDATION.md)
record strict/UBSan/SH totals and the exact public candidate verification.

Sixty-nine current 396×224 captures cover the required matrix. [Capture index](screenshots/README.md),
[setup rows](screenshots/setup-rows-sheet.png), [phases](screenshots/thinking-phases.png),
[before/after](screenshots/ui-polish-before-after.png) and [profile crop](screenshots/player-icons.png)
are actual renderer evidence. Screenshots supplement transition tests.

The exact public candidate repeated 300 2P + 144 mixed 3P + 144 control games
and both 1,200-ply follow-ups; all untimed fields matched beta.1.
[Regression fingerprints](ai/ui-beta2-regression.json) record the equality.
NUM GAME/SOKOBAN remained clean; DIFF EQ retained only its same pre-existing
untracked development file. No reference file was edited.

No changes occur under `src/core`, `src/game`, `src/ai`, `src/storage` or the
power implementation/ABI. Save IDs remain EASY=0, HARD=1, NORMAL=2; v1 A/B
format and Undo/Restart/RNG rules remain byte-identical. The UI screen enum
is not serialized. Reference projects are read-only and excluded from public assets.
All [32 calculator checks](HARDWARE_RETEST.md) are **HARDWARE TEST REQUIRED**.
