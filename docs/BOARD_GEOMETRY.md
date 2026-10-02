# Exact 73-point geometry

The user approved prioritizing the actual Korean 73-point star and ten-piece
camp placement over the originally requested disjoint `6×10+13` partition.
That partition does not describe this regular lattice: ten-point camps share
six boundary corners. The implementation records this rather than inventing
extra holes or a skewed board.

Let integer axial coordinates be `(q,r)`, with cube coordinate `s=-q-r`.
The board is the union of two opposite lattice triangles:

```
min(q,r,s) >= -3  OR  max(q,r,s) <= 3
```

Each triangle has 55 points. Their intersection is the radius-three hexagon
with 37 points, hence `55+55-37=73`. From top to bottom, row counts are
`1,2,3,10,9,8,7,8,9,10,3,2,1`. Coordinates are sorted by `(r,2q+r)` to assign
stable IDs. There are no hand-authored connections or per-hole switch cases.

The top camp is `{(3-i,-6+k): 0<=k<=3, 0<=i<=k}`, so it has `1+2+3+4=10`
points. Apply rotation `(q,r)->(-r,q+r)` to obtain the other five camps. Each
adjacent pair shares one boundary corner, all other pairs are disjoint.
Camp union size is `6×10-6=54`; 19 points belong to no camp. Alternatively,
the six purely exterior arms have six points each and the central hexagon has
37 points. These are different partitions of the same 73-point board.

Camp membership is a six-bit mask, preserving both memberships at a shared
corner. No active starting camps overlap: RED uses arm3 (bottom), GREEN arm1
(upper right), YELLOW arm5 (upper left). Goals are arm0, arm4 and arm2
respectively. Opposite mapping is `(arm+3)%6`. Home centroids have exactly
120-degree separation in the equilateral projection; 2P uses RED and GREEN.

The six neighbor vectors are `(1,0),(0,1),(-1,1),(-1,0),(0,-1),(1,-1)`.
Jump landing is twice that vector, requiring the intermediate node to exist.
Degree histogram is `{2:6,4:24,5:6,6:37}`, giving 180 undirected edges.
Generated all-pairs shortest graph distances serve AI evaluation. Tests check
each coordinate-derived connection, symmetric neighbors, opposite membership,
connected distances, unique coordinates and every camp's ten points.

Screen positions derive from the same coordinates: `x=198+8(2q+r)` and
`y=114+13r` in overview, with a twofold scale and clamped vertical offset in
zoom. Cardinal navigation is a separate deterministic graph: horizontal keys
prefer the same row; vertical keys select the nearest column in the next row.
Every node can reach every other node using those four keys. Zoom uses the
same graph and preserves cursor and selected-piece IDs.

Regenerate with `python3 tools/generate_board.py`; verify byte-for-byte with
`--check`. The table is shared by target rendering, host rendering, UI, game
engine and AI.

## Jump-cycle proof

During a noncapturing jump chain, let `B` be the original occupancy with the
moving piece's source cleared. At landing `v`, the complete occupancy is
`B` with the moving piece placed at `v`. No other pieces change. Therefore
any revisit to `v` creates exactly the same state, with exactly the same legal
continuations. Breadth-first visited-state pruning loses no reachable final
destination with distinct endpoints. Every landing is emitted, except the
unchanged source itself: the user explicitly approved excluding source-return
turns while permitting retracing during a chain. FIFO BFS with ascending landing IDs retains the
shortest path, then the lexicographically first path. This is an algorithmic
state-equivalence proof, not a Japanese route-retrace restriction.

## Beta.4 camp and reachability audit

The checked-in topology is byte-identical to beta.3. [Six camp overlays](screenshots/beta4/camp-audit.png)
and the [exact IDs, boundary rows and engine audit](screenshots/beta4/camp-audit.json)
record all six ten-hole memberships and all four boundary holes per camp.

Shared corners are 9 (RED GOAL / YELLOW HOME), 12 (RED GOAL / GREEN HOME),
33 (GREEN GOAL / YELLOW HOME), 39 (GREEN HOME / YELLOW GOAL),
60 (RED HOME / GREEN GOAL), 63 (YELLOW GOAL / RED HOME).

V2 checks own HOME/GOAL membership first. It then excludes active opponent
memberships. The same physical node can therefore be legal for two colors.
All 50 active HOME starts across 2P and 3P can reach all ten of their own goal
holes using the actual empty-board legal step generator. Fifteen fixtures
construct and commit 10/10 wins for every active color and difficulty.

Exhaustive directed two-step geometry finds zero pairs with two allowed
endpoints and an active-opponent-camp midpoint, under this V2 policy. Thus no
realizable example requires a special midpoint exemption. The engine still
checks only midpoint occupancy, without inventing a crossing restriction.
There are 312 constructed excluded intermediate routes and 58 legal goal-exit
edges in the rule tests. Cursor navigation continues to reach all 73 nodes.
