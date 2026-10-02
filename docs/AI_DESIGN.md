# Bounded AI design — beta.4

All human, Assist and AI legality comes from the same `DgRules`-parameterized
engine. A search owns a private 73-byte board, never mutates committed state or
writes flash, and revalidates its result before the one engine commit. EASY=0,
HARD=1, NORMAL=2 remain the persisted IDs. Gameplay uses one selected difficulty;
mixed difficulty exists only in host audits.

## Immediate win and search

Every node scans its complete legal set for a nine-to-ten-goal move before
beam pruning. At the root every difficulty immediately returns that move with
zero recursive nodes and unchanged RNG. This precedes heuristic ranking,
random choice, repetition preferences and defensive scoring. Terminal wins
are evaluated before in-branch repetition.

EASY retains its historical evaluator, deterministic ordering and seeded
4:2:1 choice among up to three candidates within 70 ordering points. It advances
RNG once for ordinary choices; an immediate win bypasses RNG. The 720 historical
V1 opening/midgame choice/RNG/evaluation goldens remain exact.

| Profile | Depth | Root width | Internal beam | Node limit |
|---|---:|---:|---:|---:|
| 2P NORMAL | 2 | 24 | 8 | 1,600 |
| 3P NORMAL | 4 | 20 | 3 | 1,600 |
| 2P HARD | 4 | 32 | 16 | 12,000 |
| 3P HARD | 7 | 32 | 2 | 12,000 |
| 2P HARD, own goal count >=7 | 6 | 32 | 16 | 24,000 |
| 3P HARD, own goal count >=7 | 9 | 32 | 2 | 24,000 |

An explicit caller budget still overrides the default. Iterative deepening
publishes only completed iterations, falling back to the first ordered legal
candidate if its budget expires earlier. The static workspace reserves nine
plies; there is no heap allocation or transposition table. Cancellation polls
per root candidate and at most every 32 internal entries/nodes. THINKING and
visual replay keep their existing foreground hook and RTC cadence.

2P alpha-beta is zero-sum: own evaluation minus opponent evaluation with
symmetric coefficients. 3P MaxN carries one component per color; each player
maximizes only its own component. Color rotation tests protect symmetric
endgame geometry. GREEN and YELLOW never form a coalition. HARD retains the
existing engine-based immediate-opponent-threat ordering guard.

## Endgame evaluation

`DG_ENDGAME_GOALS=7` activates focused NORMAL/HARD evaluation. A player already
in this phase at the root remains evaluated this way after a temporary exit
inside a search branch. Reaching seven goals inside a branch also activates it.
Opening/midgame base weights and difficulty profiles otherwise stay unchanged.

The engine's landing policy defines an occupancy-free neighbor graph for each
active color. A bounded cache holds distances from its ten goal holes. An
exact Hungarian assignment matches each outside piece to a distinct still
unfilled goal, avoiding multiple pieces greedily choosing the same hole.
Opponent-occupied goals remain required targets and are counted separately
from empty goals. Distances are step-graph estimates, not exact turns: jump
availability and blockers remain the legal search's responsibility.

Goal depth comes from rotating the axial coordinates to a top-pointing camp:
the four-hole row has depth 0 and the tip depth 3. A goal piece is settled when
all adjacent deeper goal points are occupied by its own pieces. Added utility:
`600*goals - 180*assignment + 12*depth + 24*settled - 60*blocked`.
With one/two remaining holes the matching explicitly focuses the remaining
outside pieces. No piece is legally locked in the goal.

A bounded root preference penalizes an immediate goal loss by 600 (NORMAL 200)
and increased assignment distance by 120 per step (NORMAL 40). This discourages
horizon-driven evacuation while retaining rearrangements that lead to a
terminal win or avoid a terminal loss. Regression tests retain 578 legal
rearrangement choices; the policy never removes a legal engine move.

## Recent turns and precedence

`DgGame` holds at most 12 committed turns in RAM: a 32-bit occupancy/next-player
hash, actor and endpoints. It records commits once, including YOU. No cursor,
search node or animation frame updates it. It is never serialized and clears
on NEW, RESTART, successful UNDO, decode and RESUME. Hash collisions can only
influence a bounded heuristic preference, never legality or a game outcome rule.

For a nonprogressing root move, reversing that actor's last move costs 120
(NORMAL 40); revisiting a recent board costs 240 per occurrence (NORMAL 80).
Increasing goal occupancy or reducing assignment at equal occupancy overrides
those repetition preferences. Terminal utilities (+/-1,000,000), including
immediate loss avoidance, override these bounded penalties. Endgame branch
repeats receive a 160-point actor penalty; 3P applies it to that actor's own
MaxN component. No turn cap, forbidden reverse move or automatic draw is added.

Priority is terminal win / terminal loss avoidance, endgame progress, then
bounded repetition preference. Required repositioning remains legal.
[Measured results and limitations](BETA4_AUDIT.md) compare unchanged beta.3
source, new AI with V1 rules, and new AI with V2 rules. Host seconds are never
calculator timings. HARD latency and stack/heap high water remain
**HARDWARE TEST REQUIRED**.
