# DIAMOND UI conventions

`dg_render()` is shared by the native add-in and actual host captures. `draw.c`
owns clipped RGB565 primitives and the pinned gint proportional font;
`menus.c` owns PLAYER/SETUP/SETTINGS/RULES; `game_view.c` owns board/HUD/modals;
`render.c` paints measured warnings and dispatches. `app.c` changes only the
LEVEL selector's explicit EASY/NORMAL/HARD display order in this beta.

The display is 396×224. Menu headers are 24px high with normal white text;
gameplay has a compact white status row. The board viewport is x4..391,
y26..203. Active softkeys are 64×18px at x66i+1/y205 on a white y204..223 strip.
Empty slots stay blank, with no hardware F-number. RESTART is yellow,
UNDO magenta, SET green, NEXT cyan, primary actions red. Every RULES softkey
has a black background and white label. Ordinary actions use blue/white.

PLAYER has adjacent red `(YOU ARE RED)` text. RED human icons have solid fill;
GREEN/YELLOW menu icons have centered black AI text. Setup has three 122×46
LEVEL tiles on one row: green smile EASY, yellow straight-mouth NORMAL,
red angry HARD. The focus/selected fill and outline stay separate from face
color. Display order clamps while persisted IDs remain EASY=0, HARD=1,
NORMAL=2. The second row is FIRST HUMAN/AI in 2P or HUMAN 1ST/2ND/3RD in 3P.
Settings changes ASSIST only. RULES retains the original 21 strings, nine
visible lines, 17px pitch and scroll offsets 0..12.

Board pieces use a thin black outline with a fully filled RED/GREEN/YELLOW
interior. White facial marks and AI squares have been removed from board pieces.
Empty holes remain white. ASSIST ON fills legal empty holes cyan inside the
same thin outline; no outer cyan target ring or crosshair remains, including
in zoom. Blue cursor/yellow selection and representative path previews retain
their roles. Assist OFF hides destinations without changing legality or invalid
move warnings. Menu AI text is not drawn on gameplay pieces.

RED starts below, YELLOW upper left, GREEN upper right. Pixel positions derive
from the frozen axial lattice: x198+8(2q+r), y114+13r in overview, twice the
spacing in zoom with the existing pan clamp. Cardinal navigation, cursor and
selection IDs, restart/undo behavior and game rules remain unchanged.

Gameplay displays AI rather than CPU in visible labels and notices. The
thinking screen keeps the board visible. Warnings use measured red text on a
small white backing. Restart retains 244×88 confirmation geometry and result
uses a 244×104 modal; EXE/EXIT semantics are unchanged. NEW after a NORMAL
result creates another NORMAL game.

[Actual captures](screenshots/README.md) include menu/setup/gameplay, three
faces and enlarged before/after piece/Assist comparisons. [UI beta audit](UI_BETA_AUDIT.md)
records pixel contracts. All actual LCD readability, brightness and latency
checks remain [HARDWARE TEST REQUIRED](HARDWARE_RETEST.md).
