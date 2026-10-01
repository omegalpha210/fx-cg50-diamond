# Acceptance evidence — beta.3

[Visualization](AI_MOVE_VISUALIZATION.md), [color inventory](UI_COLOR_AUDIT.md),
[validation](BETA_VALIDATION.md) and [capture index](screenshots/README.md) describe
the current implementation. All physical evidence remains HARDWARE TEST REQUIRED.

The 15-file source/interface/test hash guard retains engine/game/AI/storage/power.
The 73-hole star, overlapping camp memberships, move endpoints and no-op rule,
win condition, EASY/NORMAL/HARD policies, independent 3P MaxN, order, RNG and
one-decision Undo are unchanged. All 720 EASY goldens, 192 fixed selections/
stats/RNG and 192 representative path sequences match the pre-edit baseline.
Existing tactical/legal/cancellation fixtures remain. Forty-eight smoke matches
and their one capped 1200-ply follow-up match every untimed beta.2 field; illegal
moves/crashes are zero. The existing capped case stays capped, with no new draw rule.

Storage keeps its original 24/124/208-byte v1 records, 256-byte limit, enum IDs,
Assist preference, verified replacement, A/B recovery and terminal tombstone.
Original storage tests, old EASY/HARD exact re-encode, NORMAL cold-load/Undo/
Restart/order tests and save/close-failure safety remain. Visual history is not saved.

Search cancellation leaves old committed state. Successful CPU choice now commits
exactly once before its first visual frame. Tests compare the entire committed
game, representative path, ending RNG and stats to the unanimated engine result
through every 3-tick frame. MENU/OFF/EXIT skip replay and save final state; winning
commit checkpoints the inactive tombstone immediately and defers RESULT until
replay ends. Native midnight/threshold/APO tests prove frame work does not reset
idle or allocate a timer. Unchanged power thresholds/settings and OS handoff
ordering are tested, including finite save/close-failure behavior.

2P retains GREEN; 3P retains both actual AI paths through the second search and
YOU's cursor/selection/zoom/Assist/invalid input. Legal YOU commit, NEW, confirmed
RESTART, successful UNDO and every RESUME clear them. Each AI keeps only one path.
The shared entry-row mapping, clamps, NEW/RESUME semantics, F1 global resume and
normal EXIT deselect→overview→setup matrix remain tested. Busy keys are gated;
MENU/OFF/EXIT retain foreground handling.

Renderer tests check all directed STEP/JUMP edges, all lane signs, both scales,
normalized trimming/heads, every hole against actual arrow pixels, canonical
reverse-edge lane stability and both visible colors in 48 dual-lane cases.
Full renderer comparisons preserve hole interiors, cursor marks and HUD pixels.
Existing exact softkeys, font/glyph masks, panel bounds, profile/AI icons, fills,
Assist, warnings, rule scrolling and modals remain covered. YOU replaces visible
HUMAN; internal human symbols/save fields are unchanged. All NORMAL/YELLOW text
and trail use 0xac00; YELLOW fill remains 0xfe00.

131 current common-renderer captures include every requested view, all shared
directions in both scales, and all 26 actual frames of a five-hop chosen move.
Long fixtures are synthetic engine-valid boards; shared-lane routes are explicit
geometry fixtures. Screenshots are host pixels, not LCD photos or native timing.
Strict Release/UBSan, SH warnings, package bytes and memory results are in
[BETA_VALIDATION.md](BETA_VALIDATION.md) and [MEMORY.md](MEMORY.md).
The exact public candidate repeats the checks and matches local binary/captures;
uploaded assets are re-downloaded and compared. [Publication policy](PUBLICATION.md).

All [46 hardware checks](HARDWARE_RETEST.md), including the original 32 and new
14 move/trail/color/text checks, remain PENDING. No host test measures native
latency, timer cadence, BFile faults, dim/APO, physical input or total stack/heap.
