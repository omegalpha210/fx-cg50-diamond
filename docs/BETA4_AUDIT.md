# Beta.4 rules, endgame and icon audit

Date: 2026-10-03. The actual 73-node topology, ten equal pieces per participant,
2P RED/GREEN, independent 3P MaxN, no capture, optional chained jumps and distinct
final endpoints are preserved. Other calculator projects were read-only.

## Rules and legacy saves

The owner supplied the physical Korean manual's opponent-camp restriction and
confirmed all four boundary-row holes belong to a camp. [Source support and
project interpretation](RULE_SOURCES.md) are explicitly separated. The owner
then resolved the six shared corners: own HOME/GOAL membership permits entry;
otherwise active opponent HOME/GOAL forbids entry. A hole has no global owner.
Inactive YELLOW camps are open in 2P. Every intermediate jump landing is checked.
Cursor navigation and legal goal exits remain unrestricted by this policy.

The engine's single policy-parameterized generator supplies human input,
Assist and all AI levels. CPU choices are revalidated before commit. The red,
nonmodal OPPONENT CAMP notice has overview/zoom captures. All 50 active HOME
starts reach every one of their ten GOAL holes. All six shared nodes and every
four-hole boundary row are audited. [Camp IDs and overlays](BOARD_GEOMETRY.md)
and [actual UI captures](screenshots/beta4/rules-endgame-sheet.png) are reproducible.

NEW uses V2. V1 active saves retain their exact board, turn/order, difficulty,
RNG and unrestricted movement, including newly forbidden placements. RESTART
preserves that game's revision; NEW replaces it with V2. Active V1 re-encoding
remains exact. V2 adds a revision byte without changing 24/124/208-byte record
sizes, enum IDs, A/B transaction or 256-byte decoder limit. [Format](STORAGE_FORMAT.md).

## Endgame changes and controlled evidence

Every difficulty selects an immediate legal win before RNG, heuristic pruning
or recursive search. NORMAL/HARD use exact one-to-one assignment to still
unfilled goal holes, policy-aware step-graph distances, coordinate-derived goal
depth and settled pieces from seven goals onward. HARD alone adds two plies
and raises its default endgame budget from 12,000 to 24,000 nodes. Legal goal
rearrangements remain available. A 12-turn RAM-only history discourages
nonprogressing reversals/repeated positions and is cleared on lifecycle reset.
No game turn cap or repetition draw rule is introduced. [Design and weights](AI_DESIGN.md).

The comparator uses 60 deterministic synthetic positions (7/8/9 goals, all active
colors, 2P/3P), with 180 selections per version. The before source matches the
public beta.3 core/AI; the new AI is measured both under V1 and V2. These are
constructed valid positions, not claims of reachability from NEW. The table's
assignment metric is the same unrestricted-lattice one-to-one step-distance
proxy for every version; production V2 uses the stricter allowed graph.

| Version | HARD goal exits / 60 | HARD assignment regressions / 60 | Immediate win misses, all levels |
|---|---:|---:|---:|
| beta3-v1 | 2 | 2 | 0 |
| beta4-v1 | 0 | 2 | 0 |
| beta4-v2 | 0 | 1 | 0 |

After measuring the baseline, acceptance thresholds were set to zero HARD
exits and at most one assignment regression in the V2 60-position suite. Both
pass. Independent automated tests compare Hungarian assignment with a separate
subset-DP oracle, rotate colors/geometry, verify 20 last-hole progress/reversal
choices, retain 578 legal rearrangements and test 15 immediate-win cases plus
three GREEN wins immediately after RED vacates its shared last goal hole.

Five historical stalled seeds were replayed to the same diagnostic 1,200-ply
cap. Before: none finish. New AI with V1: all five finish in 61–84 plies.
New AI with V2: all five finish in 57–82 plies. The old traces expose repeated
9↔10 and 33↔41 goal shuffles, 46↔32 goal exits, and 26↔25 / 5↔12 oscillations.
The former nearest-hole evaluation lacked unique target assignment and any
cross-turn history; selective horizons could repeatedly favor these shuffles.
This diagnosis is supported by the traces, not asserted as an optimal-play proof.

| Version | HARD endgame reversals / own selections | Rate | HARD goal exits |
|---|---:|---:|---:|
| beta3-v1 | 2188 / 2294 | 95.38% | 132 |
| beta4-v1 | 1 / 48 | 2.08% | 0 |
| beta4-v2 | 1 / 30 | 3.33% | 0 |

Trajectories have different lengths because new games finish sooner; both
counts and normalized rates are shown. This is a targeted regression set, not
a general difficulty estimate. [Machine-readable metrics and timings](ai/beta4/endgame-summary.json)
and per-move CSVs include nodes, completed depth, goal counts, assignment,
exits, missed wins, reversals and host CPU seconds.

## V2 matched games

All selections are generated, revalidated and committed through V2's engine.
25 seeds × both colors × both first players give 100 2P games per pairing.
Eight seeds × six assignments × three human slots give 144 mixed 3P games.
The separate 144-game NORMAL/EASY control also finishes. Illegal moves, forbidden
landings, crashes, recurrence stops and cap timeouts are all zero: 588/588 finish.

| 2P pairing | Higher-level wins | Lower-level wins |
|---|---:|---:|
| NORMAL / EASY | 74 | 26 |
| HARD / NORMAL | 94 | 6 |
| HARD / EASY | 100 | 0 |

3P mixed wins: EASY 22, NORMAL 50, HARD 72 (144 games). The NORMAL/EASY control
is 126:18. [2P data](ai/beta4/tournament.csv), [3P data](ai/beta4/three-player.csv),
[control data](ai/beta4/three-control.csv). These support the practical
EASY < NORMAL < HARD trend in this harness. They are not human Elo or proof of
optimal play. All timing is host CPU time; calculator latency remains unmeasured.

## Icon, trails and USB

Both 92×64 icons apply the same affine scale 0.92 on both axes to the original
artwork, centered horizontally and placed at top y=1. Normal/selected nonwhite
bounds are `(6,1,86,47)` (exclusive right/bottom): one clear top row and 17 clear
bottom rows. Beta.3 had 12 bottom rows; the previous local 2px shift had 14.
90%, 92% and 94% were compared; 92% gives an extra row over 94% while keeping
more artwork than 90%. No independent width/height scaling or color redesign.
[Native comparison](screenshots/icon-before-after-1x.png),
[8× nearest-neighbor comparison](screenshots/icon-before-after-8x.png),
[label mock](screenshots/icon-label-safe-mock.png), [bounds](screenshots/icon-bounds.json).
OS label clearance remains HARDWARE TEST REQUIRED; the mock is not a device photo.

DIFF EQ/SOKOBAN references were read-only. Their normal-image bounds are
`(3,3,88,50)` / `(6,2,86,42)`, with 14 / 22 white bottom rows. Their selected
images paint the full canvas, so full-image nonwhite bounds are not artwork
bounds. The reference comparison stays private in ignored build output.

Trails retain bright yellow 0xf5c0 and light green 0x5644, 2px/3px strokes,
existing arrowheads and ±3px shared lanes. ASSIST stays cyan; YELLOW/NORMAL text
shares 0xac00. Replay remains 15 RTC ticks = 117.1875ms per hop. Final AI moves
commit once before replay; system requests checkpoint the final state and skip
replay; winning results appear afterward. Trails never enter saves.

USB detector, native lifecycle, power, save transaction and trail geometry
sources are byte-identical to the previous local candidate, protected by the
13-file preservation manifest. USB remains experimental; hardware-disabled
clock state may prevent detection. No new low-level USB lifecycle change is
included. The user explicitly authorized this experimental beta.4 binary
release with hardware checks pending; no USB-fix claim is made.

## Validation and publication

Strict host, full UBSan, strict SH, package verification and exact public-source
rebuild are release requirements. [Validation](BETA_VALIDATION.md),
[memory](MEMORY.md), [publication provenance](PUBLICATION.md) and the
[hardware checklist](HARDWARE_RETEST.md) record final results. ASan remains
NOT VERIFIED because the previously observed Darwin runtime hangs before main.
Hardware label overlap, display colors, native AI latency, total stack/heap,
USB prompts, MENU/OFF and long-play behavior remain HARDWARE TEST REQUIRED.
