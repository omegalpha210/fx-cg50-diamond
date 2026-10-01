# DIAMOND beta.2 UI conventions

The 396×224 display uses the same `dg_render()` on host and native. `draw.c`
owns clipped integer RGB565 primitives and the pinned gint proportional font;
`menus.c` owns PLAYER/SETUP/RULES; `entry.c` shares setup row identity with the
controller; `game_view.c` owns board/HUD/modals; `render.c` measures warnings.
There is one renderer. Normal text has an 11px font cell and measured glyph advances.

PLAYER uses two tiles. RED icons have a white head/shoulder silhouette;
GREEN/YELLOW icons retain centered black AI. F1 RESUME is present only for a
valid unfinished archive, independent of the selected 2P/3P tile. F2/F3/F5
are blank, F4 RULES is black/white, F6 NEXT is cyan/black.

GAME SETUP follows NUM GAME's numbered entry rows, normal font, row focus,
measured option outlines/underlines and OPEN routing. Matching unfinished
archives add RESUME at the top; otherwise NEW GAME is first. DIFFICULTY,
FIRST (2P) or HUMAN (3P), then ASSIST follow. ASSIST is always last. Four rows
use y34/34px pitch/29px height; five rows use y30/27px pitch/25px height.
Number x20, label x43 and option area x146..373 align on every row. Option
text begins 5px inside its measured width+10 box; the selected 21px box has
an outline and 2px underline. Row focus remains separate from option selection.

UP/DOWN wraps row focus, as in the inspected NUM GAME controller. LEFT/RIGHT
clamps EASY/NORMAL/HARD, HUMAN/AI, 1ST/2ND/3RD and OFF/ON. Action rows ignore
LEFT/RIGHT. EXE/F6 OPEN resumes only on RESUME; all other rows start NEW with
current values. Saved difficulty/turn order/CPU order/seed/RNG/undo are restored
exactly; current setup selectors cannot overwrite them. ASSIST remains a global
preference in archive byte 20 and can apply to a resumed game. Leaving setup
checkpoints a dirty preference. F1/F2/F3/F5 stay blank; F4 RULES and F6 OPEN
remain. The ASSIST-only SETTINGS screen and SET routes have been removed.

EASY and HUMAN/RED text use the existing green/red piece colors. NORMAL and
YELLOW actor text use NUM GAME's readable gold RGB5(25,12,0), rather than the
bright piece fill. TURN prefix is dark and actors colored; the entire centered
mode/difficulty string is green/gold/red. Goal names and every fraction glyph
share the actor color, with two rows in 2P and three in 3P. Goal dots are gone.

Board geometry, fills, thin black outlines, cyan legal endpoints, blue cursor,
yellow selection, paths and zoom/pan transform are unchanged. Board clipping
is x4..391/y26..203. The complete overview board bbox, including every possible
cursor tick, is [115,282)×[27,202). All status/warning backing boxes stay outside
it with at least 2px space. Text panels use measured width+4 and height 15;
THINKING reserves the maximum 75×15 at (6,57) with fixed origin (8,59).
Warnings wrap normal text into at most four measured lines in the left gutter,
with a maximum 107×54. Zoom permits overlap using these same compact bounds.

THINKING cycles '.', '..', '...' every 40 existing RTC ticks (312.5ms), via the
existing foreground AI cancellation hook. It clears/redraws only its fixed box.
No spinner, per-move timer, RNG/search mutation, flash write or idle reset is
introduced. Busy EXIT cancels/checkpoints directly to SETUP, including zoom.
Normal EXIT deselects, then returns zoom to overview, then checkpoints to SETUP.
Restart EXIT cancels; RULES EXIT returns to its parent; result EXIT views the board.

Softkeys occupy 64×18 at x66i+1/y205 on the white y204..223 strip. Empty slots
stay blank without F-numbers. Gameplay RESTART is yellow, UNDO magenta, primary
actions red, ordinary actions blue, RULES black/white. SETUP OPEN uses the
NUM GAME dark/white convention. RULES retains all 21 strings and scroll geometry.
Restart/result retain their 244×88 / 244×104 modal boxes and existing game semantics.

[UI polish audit](UI_POLISH_BETA2.md), [captures](screenshots/README.md) and
[hardware retest](HARDWARE_RETEST.md) provide evidence and physical test limits.
