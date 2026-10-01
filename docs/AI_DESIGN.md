# Bounded AI design

AI move legality comes only from `dg_generate()` / `dg_find_move()`. All levels
share one evaluator and one static search workspace. Searches modify a private
73-byte board; they never call `dg_commit()`, write storage, create undo, or
mutate the actual game. The chosen move is validated against the committed
board again. The caller applies returned RNG only when the move commits.

## Profiles

`DgAiProfile` contains node budget, maximum ply depth, root candidate width and
internal beam. `dg_ai_profile()` separates persisted IDs from display order.
The [difficulty audit](AI_DIFFICULTY_AUDIT.md) records final parameters, measured
choices, strength results and why the initial profiles were adjusted. All
production three-player AIs use the one globally selected difficulty; mixed
levels exist only in host audit tools.

| Level | 2P algorithm | 3P algorithm |
|---|---|---|
| EASY | Heuristic / seeded top-candidate choice | Same policy |
| NORMAL | Small bounded alpha-beta | Small bounded MaxN |
| HARD | Deeper bounded alpha-beta | Deeper bounded MaxN |

A four-ply horizon in 3P includes the player's next move after both opponents
reply. NORMAL uses a smaller root set and node budget. HARD reaches seven plies
with a narrow internal beam, including the player's third move. Search width as
well as depth matters; the audit measures actual choices
and outcomes rather than inferring difficulty from depth alone.

## EASY baseline

The original evaluator, move ordering, deterministic tie-breaks and weighted
random policy are preserved. Every legal destination is ranked by own heuristic
score plus a small hop bonus. Choose among the best three, excluding candidates
more than 70 ordering points behind the best, with weights 4:2:1. A winning move
reduces the set to one. Consume the gameplay RNG once per choice, including a
single-candidate choice. Twenty pinned 36-ply traces protect 720 exact choices,
returned RNG values and evaluation results over both modes and every human slot.

The evaluator rewards goal occupancy, evacuation of home, goal packing and
mobility, and penalizes graph distance to available goal holes and lingering in
other target camps. These are soft heuristics; they add no movement restriction.
Coordinates, distances and camp membership come from the shared board tables.

## Shared search

NORMAL/HARD use iterative deepening and publish only a completed iteration.
Budget exhaustion returns the last completed choice, or the highest ordered
legal candidate if no iteration completed. The board is restored while branches
unwind. Scores/ties are deterministic and these profiles do not advance RNG.

2P alpha-beta alternates saved RED/GREEN order and uses the root score minus
opponent score. Terminal root/opponent wins are +1,000,000 / -1,000,000. A repeated
board with the same turn inside the searched branch receives utility zero; this
is a search evaluation policy, never a game draw or prohibition.

3P MaxN returns independent per-color utilities. Each turn selects the child
maximizing that current player's own component. GREEN/YELLOW have no alliance;
no two-player pruning bound is applied. Winner utility is +1,000,000 and each
other component -1,000,000. Saved order is respected.

HARD root ordering also recognizes a player's immediate nine-goal win threat
using `dg_find_move()`, and retains legal defensive blocking candidates in its
beam. A fixed-position audit exposed a lost defensive option in the older
ordering. EASY/NORMAL evaluation and ordering remain unchanged by this guard.
The guard does not reject any legal move or replace engine validation.

## Cancellation and memory

Root candidate ordering polls cancellation per candidate; internal ordering and
visited nodes poll at most every 32 entries. The foreground callback observes
physical input and power deadlines. Cancellation returns no move and leaves
committed board/turn/RNG/undo unchanged. Pending animation is also uncommitted;
MENU/OFF/APO checkpoint the last committed state.

There is no heap allocation or transposition table. Legal move storage and
per-depth candidate/ancestor arrays are static and reserve seven plies.
The host reference uses a larger search budget and wider beams. Reference code
is compiled only under `DG_HOST` and is never available in production UI.
`dg_ai_workspace_bytes()` measures the compiled workspace; `tt_hits` is zero.
This workspace is intentionally non-reentrant, used by one foreground search.

## Evidence limits

The host reference scores every legal forced root move at a larger fixed
search budget and compares the chosen move to its best utility. For 3P, regret
uses the current player's component. This shares the production evaluator and
uses selective beams; it is a correlated proxy, not an optimal-play oracle.

Matched seeded tournaments swap colors/first player and rotate three-player
assignments. Diagnostic recurrence/cap stops belong only to the harness; the
actual game has no turn limit or added draw rule. 3P goal progress and rank
proxies are not Elo. Host timings and process RSS do not measure calculator
latency or native heap. Final HARD/NORMAL latency and long-play high-water
measurements remain [HARDWARE TEST REQUIRED](HARDWARE_RETEST.md).
