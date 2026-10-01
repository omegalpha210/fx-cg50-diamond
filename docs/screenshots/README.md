# Actual renderer captures

These 396×224 PNGs come from `tools/capture.c` calling the same `dg_render()`
used by the native add-in. They are host renders, not hardware photographs or
separately drawn UI mockups. The current driver produces 37 frames, including
compatibility aliases for the original setup and thinking/animation filenames.

LEVEL display order is EASY / NORMAL / HARD, with stable saved IDs 0 / 2 / 1.
The renderer shows full-color board pieces, solid cyan Assist destinations,
black/white RULES, the red human annotation and visible AI terminology.
Actual search profiles are described in [AI_DESIGN.md](../AI_DESIGN.md);
[UI_BETA_AUDIT.md](../UI_BETA_AUDIT.md) records this visual change and pixel evidence.
Control semantics are in [UI_CONVENTIONS.md](../UI_CONVENTIONS.md).
Calculator-only validation remains [HARDWARE TEST REQUIRED](../HARDWARE_RETEST.md).

| Screen | Actual common-renderer PNG |
| --- | --- |
| PLAYER / 2P tile selected | [3P selected](player.png), [2P selected](player-2p.png) |
| 2P SETUP, each level | [EASY](setup-2p-easy.png), [NORMAL](setup-2p-normal.png), [HARD](setup-2p-hard.png) |
| 3P SETUP, each level | [EASY](setup-3p-easy.png), [NORMAL](setup-3p-normal.png), [HARD](setup-3p-hard.png) |
| All three faces, enlarged actual crop | [Level faces](level-faces.png) |
| SETTINGS / RULES | [Settings](settings.png), [Rules](rules.png), [Controls page](rules-controls.png) |
| Overview | [2P](2p-overview.png), [3P](3p-overview.png) |
| Piece readability fixture | [Overview](piece-readability.png), [Zoom](piece-readability-zoom.png) |
| Selected piece | [Selection](piece-selection.png) |
| Assist destinations | [ON](assist-on.png), [OFF](assist-off.png) |
| Multi-jump and zoom | [Multi-jump](multi-jump.png), [Zoom](zoom.png), [Zoom Assist OFF](zoom-assist-off.png) |
| AI states | [Thinking](ai-thinking.png), [Animation](ai-animation.png) |
| Invalid move warning | [Warning](warning.png) |
| Restart and undo | [Restart](restart.png), [Undo available](undo-ready.png), [Move with undo](move-with-undo.png) |
| Result | [Human](result.png), [NORMAL AI](result-normal.png), [GREEN AI](result-green-ai.png), [YELLOW AI](result-yellow-ai.png) |
| Frozen final board / active resume | [Final board](result-board.png), [Resume](player-resume.png) |

[Contact sheet](contact-sheet.png) and [level selection sheet](level-selection-sheet.png)
place actual frames without redrawing their pixels.
[Piece before/after](piece-before-after.png) includes all three piece colors,
white empties and cyan markers in both view scales; its measured
[fill pixel counts](piece-fill-metrics.csv) use unchanged interior radii.
[Full-frame comparison](beta-before-after.png) uses only the
[frozen own baseline](before-fill/README.md).

Regenerate after a host build with `python3 tools/ui_captures.py`. The public
checkout needs its own source, host capture executable, Pillow and retained own
baseline PNGs; it does not need the reference projects, private screenshots or
historical validation directories. C renders every app pixel and exports crop
coordinates from the real geometry. Python converts PNGs and assembles nearest
neighbor crops/contact sheets. Native dim, brightness restoration, APO,
readability and response time still require hardware retesting.
