# Current beta.4 difficulty evidence

V2 matched games: NORMAL/EASY 74:26, HARD/NORMAL 94:6, HARD/EASY 100:0.
3P mixed wins are EASY 22, NORMAL 50, HARD 72. All 588 matched/control games
finish with zero illegal moves or crashes. Five old 1,200-ply stalled seeds
now finish under both new-AI V1 and V2. See [complete beta.4 audit](BETA4_AUDIT.md)
for metrics, exposure-normalized reversals and evidence limits.

The tables below remain historical measurements; their old stalled outcomes
and parameters do not describe beta.4's focused endgame policy.

# Historical beta.1 difficulty audit

The final profiles show a practical EASY → NORMAL → HARD strength trend in
matched host games. The bounded reference is correlated with the production
evaluator and its nonterminal 3P scores do **not** establish a monotonic
hierarchy. Five diagnostic games remained unfinished at 1,200 plies. These
limitations remain part of the result; there is no gameplay turn cap or
repetition draw rule.

## Profiles and preserved behavior

Serialized IDs remain EASY=0, HARD=1, NORMAL=2. UI display order is independent.
A single search engine and fixed workspace serve all three levels.

| Mode / level | Algorithm | Maximum plies | Root candidates | Internal beam | Default visited-node budget |
| --- | --- | ---: | ---: | ---: | ---: |
| EASY, both modes | Existing weighted heuristic | 1 | 32 | Top 3, score window | No recursive search |
| 2P NORMAL | Alpha-beta minimax | 2 | 24 | 8 | 1,600 |
| 2P HARD | Alpha-beta minimax | 4 | 32 | 16 | 12,000 |
| 3P NORMAL | Independent MaxN | 4 | 20 | 3 | 1,600 |
| 3P HARD | Independent MaxN | 7 | 32 | 2 | 12,000 |

EASY's evaluation, ordering, 70-point candidate window, 4:2:1 weighted choice,
and returned RNG are unchanged. Twenty golden games cover every human turn
slot in both modes, four seeds per slot, and 36 plies each: 720 preserved
choice/evaluation/RNG checks. All 64 fixed-position EASY choices also match
[the baseline dataset](ai/baseline-choices.csv), including returned RNG.

Before this change, HARD used 2P depth 4 / beam 16 / 12,000 nodes and 3P depth
3 / beam 10 / 8,000 nodes, with 32 root candidates. The 2P depth/budget remains
unchanged. HARD now retains immediate-win defensive blocks in root ordering:
recognition of a nine-goal opponent uses the engine's own move validator.
This fixes one constructed defensive fixture whose necessary block was
previously omitted from the beam. A direct own win still has priority.

3P HARD changed 21 of 32 fixed choices; 2P HARD changed one. Both 3P levels
maximize the acting player's own RED/YELLOW/GREEN component. Equal CPU
profiles do not make those players a team. Every generated and published
move still comes from, and is revalidated by, the frozen engine. Camp access,
source-return prohibition, captures, winning, undo and RNG semantics were
not changed.

## Why the profiles changed

Initial 2P NORMAL depth 2 already won 27/32 pilot games against EASY; existing
2P HARD won 29/32 against NORMAL. Shallow 3P NORMAL depth 3 did not separate
well: its balanced EASY-only control finished NORMAL 76 / EASY 68, and it
matched old HARD's fixed-position average regret exactly.

Moving 3P NORMAL to a narrow four-ply MaxN includes the player's next own
move. The final balanced control won NORMAL 126 / EASY 18. However, four-ply
HARD with a wider beam then matched NORMAL's reference regret on all 32
positions and produced only a small mixed-game advantage. Seven-ply HARD
with beam 2 includes a third own move while bounding completed iterative
search to at most 7,904 visited nodes when all 32 roots have full beams.
The resulting final mixed games and larger reference distinguish HARD from
NORMAL. This is deeper **selective** search; a narrow beam can miss good
branches. Depth alone is not the strength evidence.

## Fixed-position quality

The suite has 32 positions per mode: 27 reachable positions and five
constructed engine-valid tactical positions. Reachable positions include
both initial actors and seeded EASY play at 2/4/6/8/10/12/16/20 rounds. Packed
initial homes and early evacuation cover home congestion. Other fixtures
cover long jumps, goal congestion, central blocking, goal entry, immediate
winning and defensive threats. Three-player occupied central boards and
reachable RED/YELLOW/GREEN interactions exercise independent objectives.
Constructed boards have ten pieces per participant but are not claimed to
be reachable from NEW.

A host-only reference scores **every legal root move** separately at a common
horizon: 2P alpha-beta depth 5 / beam 20, and 3P MaxN depth 7 / beam 3. Each
forced move has a 1,000,000-node guard; all reference calls finished. In 3P the
score is the current player's own component. Regret is reference-best utility
minus the selected move's reference utility. The reference shares the same
heuristic and uses selective beams: it is a proxy, not an oracle or proof of
human difficulty. EASY contributes one seeded selection per position.

| Mode | Level | Mean regret, all 32 | Common nonterminal mean, 30 | Reference-best matches |
| --- | --- | ---: | ---: | ---: |
| 2P | EASY | 31,216.09 | 96.73 | 13 |
| 2P | NORMAL | 31,197.38 | 76.77 | 15 |
| 2P | HARD | 35.84 | 38.23 | 18 |
| 3P | EASY | 31,369.75 | 152.47 | 10 |
| 3P | NORMAL | 214.56 | 228.87 | 7 |
| 3P | HARD | 154.19 | 164.47 | 14 |

The nine-goal defensive-threat fixture dominates whole-suite means: EASY and
NORMAL both miss the 2P defensive block; EASY alone misses it in 3P. The
common nonterminal subset excludes both that fixture and the immediate-win
fixture for every level, using a shared utility-magnitude threshold of
900,000. In this subset, 3P NORMAL and HARD do not beat EASY's average proxy
regret. The selected benchmark is therefore not sufficient by itself to
claim monotonic strength; matched games provide separate practical evidence.

| Mode | Comparison | Improved | Tied | Worse |
| --- | --- | ---: | ---: | ---: |
| 2P | NORMAL versus EASY | 6 | 22 | 4 |
| 2P | HARD versus NORMAL | 9 | 19 | 4 |
| 3P | NORMAL versus EASY | 8 | 14 | 10 |
| 3P | HARD versus NORMAL | 12 | 14 | 6 |

Detailed [per-position scores](ai/quality-positions.csv),
[quality summaries](ai/quality-summary.csv), and
[comparisons](ai/quality-comparisons.csv) preserve the evidence.

## Matched games

The 2P tournament uses 25 fixed seeds, both first actors and both level-to-color
assignments for each pairing: 100 games per pairing, 300 total. Each matched
pair starts with the same eight-ply seeded EASY opening, providing diversity
for deterministic NORMAL/HARD comparisons. Levels then follow their assigned
colors. All moves use the real engine and are checked before commit.

| Pairing | Higher-level wins | Lower-level wins | Repetition halts | Cap timeouts | Mean plies |
| --- | ---: | ---: | ---: | ---: | ---: |
| NORMAL / EASY | 85 | 15 | 0 | 0 | 64.39 |
| HARD / NORMAL | 90 | 8 | 2 | 0 | 68.84 |
| HARD / EASY | 98 | 2 | 0 | 0 | 56.90 |

The 3P mixed tournament uses eight seeds, all six EASY/NORMAL/HARD color
assignments, and all three human slots: 144 games. The saved CPU order is
seeded normally. A nine-ply seeded EASY opening precedes each comparison.

| Level | Wins / 144 | Mean goal occupancy | Mean goal-rank proxy |
| --- | ---: | ---: | ---: |
| EASY | 26 | 8.278 | 2.378 |
| NORMAL | 46 | 9.118 | 1.847 |
| HARD | 69 | 9.160 | 1.774 |

Three games reached the 400-ply diagnostic cap. The goal-rank proxy orders
players by occupied goal holes and averages tied ranks; lower is better.
This is not 2P Elo or a complete final placement after the first winner.

A separate balanced 144-game 3P control rotates one NORMAL plus two EASY
players and one EASY plus two NORMAL players through every color and human
slot. Each level has 216 player exposures. NORMAL wins 126 games versus
EASY's 18, with mean goal occupancy 9.477 versus 7.940 and rank proxy 1.535
versus 2.465. No control game hit the cap. This control addresses HARD's
preemption of finishes in the mixed tournament.

All 588 final tournament/control games had zero illegal selections. Diagnostic
repetition halts require equal occupancy, turn and RNG. The two 2P recurrence
cases and all three capped 3P cases were replayed from their original seeds
without early recurrence termination to 1,200 plies. **All five remained
unfinished.** The 2P cases were 9/10 versus 9/10; 3P cases had goal counts
9/8/8, 9/9/7, and 7/9/9. These are recorded stalled trajectories, not hidden
successes or automatic gameplay draws.

See [2P summaries](ai/tournament-summary.csv),
[3P/control summaries](ai/three-player-summary.csv), and
[extended-cap follow-up](ai/recheck-summary.csv).

## Host work, memory and validation

Across the 32 positions per mode, mean visited nodes were:

| Mode | EASY | NORMAL | HARD |
| --- | ---: | ---: | ---: |
| 2P | 50.13 | 76.75 | 2,742.34 |
| 3P | 50.47 | 1,066.16 | 6,887.59 |

Mean host CPU times were approximately 0.013 / 0.187 / 6.547 ms for 2P
EASY/NORMAL/HARD and 0.012 / 2.376 / 24.273 ms for 3P. These are observations
from this host and build, not calculator latency. Early terminal wins finish
at depth 1; other completed depths reach the configured limit. Before/after
counts, selected moves and timings are in [baseline comparisons](ai/baseline-comparison.csv)
and [current metrics](ai/choice-metrics.csv).

The target workspace increased from 5,261 to 6,257 bytes: +996 bytes, about
19%. There is one static workspace, no heap allocation and no TT. Search
buffers and ancestor storage are bounded for seven plies in both host and
target builds. Cooperative cancellation remains checked at every root
candidate and at most every 32 search nodes; budgets return only a completed
iteration or a validated legal fallback. The target's largest reported stack
frame remains 1,344 bytes. Full application memory measurements are maintained
separately in the beta memory report.

Strict AI unit tests and UBSan passed, including EASY goldens, all three
profiles, tiny budgets, deterministic NORMAL/HARD, cancellation and wins.
ASan is **NOT VERIFIED** in this environment: both the unit binary and a
minimal `puts` probe stalled before `main` in ASan shadow-memory initialization
and its static spin mutex. This was a runtime failure, not a test pass.

## Reproduction

Build the host targets with `DG_HOST=ON`, then run:

```sh
mkdir -p build/ai
build/host/ai_strength --choices > build/ai/final-choices.csv
build/host/ai_strength --quality > build/ai/final-quality.csv
build/host/ai_strength --tournament 25 400 > build/ai/final-tournament.csv
build/host/ai_strength --3p 8 400 > build/ai/final-three.csv
build/host/ai_strength --3p-control 8 400 > build/ai/final-three-control.csv
build/host/ai_strength --recheck build/ai/final-tournament.csv 1200 > build/ai/tournament-recheck.csv
build/host/ai_strength --recheck build/ai/final-three.csv 1200 > build/ai/three-recheck.csv
python3 tools/summarize_ai.py \
  --choices build/ai/final-choices.csv \
  --baseline docs/ai/baseline-choices.csv \
  --quality build/ai/final-quality.csv \
  --tournament build/ai/final-tournament.csv \
  --three build/ai/final-three.csv \
  --control build/ai/final-three-control.csv \
  --rechecks build/ai/tournament-recheck.csv build/ai/three-recheck.csv \
  --out docs/ai
```

Raw per-game logs stay under `build/`; only curated choices, quality datasets
and aggregate reports are published. Exact-source reruns should reproduce
choices, regret, nodes and outcomes. Timing/RSS can vary.

**HARDWARE TEST REQUIRED:** NORMAL 2P/3P response time, HARD 2P/3P response
time, cancellation during thinking, repeated long play, and actual user
perception of the three levels. No host measurement establishes the intended
calculator latency.
