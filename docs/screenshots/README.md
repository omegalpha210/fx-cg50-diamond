# Actual beta.3 renderer captures

131 396×224 frames come from `tools/capture.c` calling the same
`dg_render()` used by the add-in. These are actual host-rendered UI pixels;
Python converts PPM to PNG and assembles sheets/crops without drawing a second UI.
Captions use the retained gint atlas; enlarged crops use nearest-neighbor pixels.
[Conventions](../UI_CONVENTIONS.md), [visualization](../AI_MOVE_VISUALIZATION.md) and
[hardware retest](../HARDWARE_RETEST.md) explain behavior and physical limits.

| View | Actual captures |
|---|---|
| PLAYER no resume | [3P tile](player.png), [2P tile](player-2p.png), [profile/AI crop](player-icons.png) |
| PLAYER valid global resume | [Saved 2P](player-resume-2p.png), [Saved 3P](player-resume-3p.png) |
| PLAYER tile mismatch | [Saved 3P / tile 2P](player-resume-saved-3p-tile-2p.png), [Saved 2P / tile 3P](player-resume-saved-2p-tile-3p.png) |
| Numbered setup, no resume | [2P](setup-2p-no-resume.png), [3P](setup-3p-no-resume.png) |
| Numbered setup, matching resume | [2P](setup-2p-resume.png), [3P](setup-3p-resume.png), [four-layout sheet](setup-rows-sheet.png) |
| Mismatched setup | [2P](setup-2p-mismatch.png), [3P](setup-3p-mismatch.png) |
| 2P difficulty selected | [EASY](setup-2p-easy.png), [NORMAL](setup-2p-normal.png), [HARD](setup-2p-hard.png) |
| 3P difficulty selected | [EASY](setup-3p-easy.png), [NORMAL](setup-3p-normal.png), [HARD](setup-3p-hard.png) |
| FIRST selection | [YOU](setup-2p-first-human.png), [AI](setup-2p-first-ai.png) |
| YOU slot selection | [1ST](setup-3p-human-1st.png), [2ND](setup-3p-human-2nd.png), [3RD](setup-3p-human-3rd.png) |
| ASSIST last row | [2P OFF](setup-2p-assist-off.png), [2P ON](setup-2p-assist-on.png), [3P OFF](setup-3p-assist-off.png), [3P ON](setup-3p-assist-on.png) |
| Colored HUD and goal fractions | [2P EASY / YOU](hud-human-2p-easy.png), [3P NORMAL / YELLOW AI](hud-yellow-3p-normal.png), [3P HARD / GREEN AI](hud-green-3p-hard.png) |
| Thinking overview | [1 dot](thinking-1.png), [2 dots](thinking-2.png), [3 dots](thinking-3.png), [phase sheet](thinking-phases.png) |
| Thinking zoom | [1 dot](thinking-zoom-1.png), [2 dots](thinking-zoom-2.png), [3 dots](thinking-zoom-3.png) |
| Overview | [2P](2p-overview.png), [3P](3p-overview.png), [selected](selected-overview.png) |
| Zoom | [unselected](zoom-unselected.png), [selected](selected-zoom.png), [multi-jump](zoom.png), [Assist OFF](zoom-assist-off.png) |
| Piece fill fixtures | [overview](piece-readability.png), [zoom](piece-readability-zoom.png), [selection](piece-selection.png) |
| Assist/path | [ON](assist-on.png), [OFF](assist-off.png), [multi-jump](multi-jump.png) |
| Warnings | [invalid move](warning.png), [long overview](warning-long.png), [zoom](warning-zoom.png) |
| Rules | [first page](rules.png), [controls page](rules-controls.png) |
| Restart/undo/AI animation | [restart](restart.png), [undo ready](undo-ready.png), [move with undo](move-with-undo.png), [animation](ai-animation.png) |
| Results | [human](result.png), [NORMAL](result-normal.png), [GREEN AI](result-green-ai.png), [YELLOW AI](result-yellow-ai.png), [frozen board](result-board.png) |

[Contact sheet](contact-sheet.png) and [level sheet](level-selection-sheet.png)
contain actual frames. [UI beta.1→beta.2](ui-polish-before-after.png) uses
[frozen beta.1 own captures](beta1-before/README.md). The earlier
[piece comparison](piece-before-after.png), [fill counts](piece-fill-metrics.csv)
and [full-frame comparison](beta-before-after.png) retain the
[before-fill baseline](before-fill/README.md). Historical compatibility aliases
setup-2p/setup-3p/cpu-thinking/cpu-animation/player-resume remain actual current renders.
Old face crops and SETTINGS current captures have been removed.

## AI move, trail and palette evidence

| Requested view | Actual common-renderer capture |
|---|---|
| YOU after two AI replies / both paths | [YOU turn](trails-you.png), [both paths](trails-both.png) |
| Long turning paths | [GREEN](green-long-trail.png), [YELLOW](yellow-long-trail.png), [YELLOW zoom](yellow-long-trail-zoom.png) |
| 2P last AI path | [GREEN only](trails-2p.png) |
| Same/opposite shared edges | [six directions/both views](shared-lanes-sheet.png), [4x exact pixel crops](shared-lane-details.png) |
| Second AI THINKING with first trail | [thinking + retained trail](trails-thinking.png) |
| Actual five-hop animation | [all 26 frames](animation-contact-sheet.png), [frame CSV](animation-frames.csv), [final trail](animation-final-trail.png) |
| Zoom, selection and Assist | [zoom](trails-zoom.png), [selected](trails-selected.png), [Assist](trails-assist.png), [Assist zoom](trails-assist-zoom.png) |
| YOU/YELLOW/GREEN TURN labels | [YOU](trails-you.png), [YELLOW](hud-yellow-3p-normal.png), [GREEN](hud-green-3p-hard.png) |
| Unified NORMAL/YELLOW | [palette sheet](yellow-palette-sheet.png) |
| YOU result | [YOU WIN](result.png) |

[Trail sheet](ai-trails-sheet.png) and [capture manifest](capture-manifest.json)
record current scope. Long positions are constructed engine-valid tactical
fixtures; actual AI search/commit/time APIs select and replay their paths.
The two-AI YOU view really commits both replies in their configured order.
Shared-edge routes are explicit geometry fixtures, not claimed AI choices.
The animation CSV records one logical commit throughout ticks 0..75; THINKING
is absent after search, actor remains GREEN during replay and changes afterward.
Captures freeze ideal RTC ticks, not actual calculator cadence.

The beta.1→beta.2 comparison now reads [frozen beta.2 own images](beta2-before/README.md),
so its historical labels/colors stay accurate. Current terminology is YOU;
legacy filename aliases containing human remain filenames only.

Regenerate after a host build with `python3 tools/ui_captures.py`. No reference
project or private image is required. Static images supplement the controller,
native-clock and pixel-transition tests; they cannot establish calculator timing.
