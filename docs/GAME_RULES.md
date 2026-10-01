# DIAMOND game rules

DIAMOND implements the Korean ten-piece game for one human and one or two independent CPUs. The human is RED. Its conventional star has 73 playable points. Each ten-point camp includes four central-hexagon boundary points, so neighboring camp memberships share a corner. The user approved this correction to the original disjoint-region requirement; see [RULE_SOURCES.md](RULE_SOURCES.md).

The user also chose distinct final endpoints: jump routes may retrace, but a
turn cannot end on its source. This project decision excludes an unchanged-board
return turn without importing a Japanese prohibition on retracing paths.

## Players and goals

Each player begins with ten equal pieces and aims to move all ten into the camp directly opposite their start. RED starts below and aims upward. GREEN starts upper right and aims lower left. In three-player games, YELLOW starts upper left and aims lower right. Two-player starts are 120° apart; three-player starts are mutually 120° apart.

Two-player setup selects HUMAN or CPU first. Three-player setup selects the human's 1ST, 2ND or 3RD slot; the two CPUs fill the other slots in random order for each new game. Restart retains that order. Resume restores the saved order and current turn. Each CPU seeks its own victory.

## One turn

Move one of your pieces using either:

- **STEP:** move to an adjacent empty point.
- **JUMP:** cross one adjacent occupied point in a straight lattice direction and land on the next empty point. Your own or another player's piece can be crossed. The crossed piece stays on the board.

After a jump, another legal jump can follow from its landing. Direction can change. You can stop after any jump, so every intermediate landing is a legal destination. A jump is optional. You cannot combine a step and a jump in the same turn. All pieces have identical abilities; there is no king or capture.

The engine lists every legal final destination once and keeps a deterministic shortest representative path for display. Revisiting the same jumping state is pruned because it cannot reveal additional destinations; there is no separate route-retrace rule.

## Camps and victory

This project permits movement anywhere on the board under the same STEP/JUMP rules, including unused and opposing camps. Pieces can leave their goal before the game ends. These are project decisions, not a claim about every Korean edition.

The first move leaving all ten of a player's pieces in that player's ten-point goal ends the game immediately. Three-player games do not continue for second or third place. Completed boards are frozen for inspection and are not unfinished resumable games.

## Assist and controls

Assist ON marks all legal final destinations of the selected human piece. It can preview one path for the focused destination; it does not display every possible route. Assist OFF hides destination markers while the same engine still rejects illegal moves. Assist is a global preference and does not change game rules.

| Screen | Controls |
| --- | --- |
| PLAYER | Arrows select 2/3 PLAYER; EXE or F6 NEXT; F1 SET; F2 RESUME if an unfinished save is valid; F4 RULES |
| GAME SETUP | Up/down focus; left/right change clamped options; F6 PLAY starts a new game; EXIT returns to PLAYER |
| SETTINGS | Left/right set ASSIST ON/OFF; EXIT returns |
| Gameplay | Arrows navigate; EXE/F6 selects/confirms; EXIT cancels selection or leaves the game; F1 RESTART confirmation; F2 UNDO on a human turn; F4 RULES; F5 overview/zoom |
| Restart confirmation | EXE YES restarts with the same configuration, CPU order and initial RNG; EXIT NO leaves the game unchanged |
| Result | EXE NEW GAME; EXIT VIEW BOARD; frozen board F6 NEW, EXIT setup |
| Everywhere | MENU checkpoints committed state and opens CASIO Main Menu; SHIFT+AC/ON checkpoints before power off |

Undo returns to the preceding human decision, including intervening CPU responses, with board, turn and RNG restored. Only one such undo is available until the human commits another move. Restart recreates the same game's initial position and order; NEW creates a fresh game and replaces the single active resume only after a successful save transaction.

See [RULE_SOURCES.md](RULE_SOURCES.md) for source-supported facts, project choices and excluded Japanese rules. Calculator input, visual readability and lifecycle behavior require hardware retesting.
