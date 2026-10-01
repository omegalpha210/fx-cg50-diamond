# Acceptance evidence — beta.2

The current UI is documented in [UI_POLISH_BETA2.md](UI_POLISH_BETA2.md).
[Beta validation](BETA_VALIDATION.md) records the exact public source rebuild;
all physical checks remain [HARDWARE TEST REQUIRED](HARDWARE_RETEST.md).

## Preserved game, AI, save and power behavior

The 73-node axial star, six ten-point camps with six shared corner memberships
and 19 points outside their union are unchanged. RED is below, YELLOW upper
left and GREEN upper right. Engine generation remains the sole legality path
for human moves, cyan Assist and AI proposals. Chains may retrace but must
finish away from their source; no captures, mixed step/jump or Japanese rules
are added. Independent coordinate-oracle testing covers 5,000 reachable boards
and 71,645 chained moves, with 25,186,841 assertions.

All 720 EASY goldens and 192 recorded EASY/NORMAL/HARD fixed choices agree
exactly with the pre-edit beta.1 baseline, including RNG/nodes/depth/beam.
The game/AI/engine/storage/power sources/interfaces and engine/AI unit sources
have 15 exact hash guards. There is no AI-strength or game-rule change.
Existing tactical/legal/determinism/cancellation/Undo/Restart tests remain.
Current tournament regression retains the previously documented strength
limitations and diagnostic cycle/cap policy; no new gameplay draw is introduced.

Storage retains 10,741 assertions for malformed/corrupted/truncated records,
reachable-state roundtrips, A/B faults/readback/recovery, generation wrap,
tombstones and actual isolated host files. Old EASY/HARD fixtures re-encode
byte-identically; NORMAL cold-load/Undo/Restart/A/B recovery tests remain.
Archive records are 24/124/208 bytes with a 256-byte decoder limit, unchanged
stored IDs 0/1/2 and global Assist byte 20. Verified NEW preserves the previous
resume until replacement succeeds. No search/cursor/visual redraw writes flash.

## Controller and native acceptance

The original app lifecycle, cancellation, one-human-decision Undo, Restart and
result tombstone tests remain. Added setup tests cover every four/five-row
order, global/local resume distinction, all level/slot/Assist clamps, action-row
no-ops, EXE/OPEN from every non-RESUME row, exact saved game restoration and
Assist persistence. NEW still uses the original seed/AI-order policy and
verified storage replacement; options cannot silently overwrite resumed game bytes.

EXIT tests establish deselect → zoom out → checkpoint to setup, including
three EXIT presses for selected+zoom and two for selected+overview. Restart,
RULES and result exits take priority. Native CPU-busy EXIT preserves the direct
cancel/save/setup path even after the search clears its thinking flag.

The actual native static functions are compiled into a host API shim. Existing
power/settings/barrier/save/close-failure checks remain, and clock-stepped
search tests compare all six profiles against a nonanimated reference. The
three dot phases run through the real cancellation hook without consuming RNG,
changing chosen moves/stats, resetting idle, writing flash or allocating a timer.
Midnight wrapping, exact phase thresholds and busy zoom EXIT are covered.

## Renderer acceptance

642 immutable frames and 2,058,781 bounded rectangle calls check normal-font
numbered rows, every option/focus layout, exact softkeys, white profile/black AI
pixels, unchanged board/Assist fill, colors on complete HUD/fractions, glyph-width
centering, actual backing rectangles and warning wrapping. The overview envelope
includes every possible cursor tick. No status box intersects it; zoom panels
are bounded. A three-dot to one-dot partial update checks every pixel for residue
and proves all pixels outside the fixed region are unchanged.

Sixty-nine actual common-renderer captures include global resume with a mismatched
tile, matching and mismatched setup, all difficulty/slot/Assist states, colored
HUD/progress examples, all thinking phases in both views, selected/unselected
views, warnings, rules, restart, undo, animation and all results. These are
host pixels, not hardware photos. [Capture index](screenshots/README.md) links them.

Strict host/UBSan/SH/package results and [memory](MEMORY.md) provide software
acceptance only. Real LCD contrast, OS values, dim/APO, MENU/OFF, persistent
BFile operation, physical key behavior, native AI latency and total runtime
stack/heap high water remain unverified until the 32-item hardware list is run.
