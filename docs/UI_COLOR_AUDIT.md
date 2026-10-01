# Beta.3 semantic color audit

The common renderer's central `src/ui/draw.h` palette supplies both host and SH.
RGB5 below means the project's `DG_RGB(r,g,b)` convention (green is shifted by
six bits into RGB565). Semantic text colors are separate from fill only where
needed; RED/GREEN retain their existing values.

| Role/token | RGB5 | RGB565 | Actual uses |
|---|---|---|---|
| PIECE_RED | 28,3,3 | 0xe0c3 | RED pieces and menu profile fill |
| UI_RED_TEXT | alias PIECE_RED | 0xe0c3 | YOU label, TURN YOU, YOU progress/result, HARD text |
| PIECE_GREEN | 0,20,8 | 0x0508 | GREEN pieces and AI icons |
| UI_GREEN_TEXT | alias PIECE_GREEN | 0x0508 | GREEN TURN/progress/result/trail/heads, EASY options and mode label |
| PIECE_YELLOW | 31,24,0 | 0xfe00 | YELLOW pieces/icons; existing yellow selection ring |
| UI_YELLOW_TEXT | 21,16,0 | 0xac00 | NORMAL option/outline/underline, saved summary, result difficulty and full mode label; YELLOW TURN/progress/result/trail/heads |
| UI_GOLD | alias UI_YELLOW_TEXT | 0xac00 | Compatibility name only, no separate color |
| BOARD_CYAN | 0,23,26 | 0x05da | Legal Assist destinations and YOU representative preview |
| BOARD_BLUE | 0,12,25 | 0x0319 | Cursor box/ticks |
| BOARD_INK | 3,5,8 | 0x1948 | Selection outer ring |
| UI_INK / UI_BLACK | 3,5,7 / black | 0x1947 / 0x0000 | TURN prefix, general text, outlines, AI icon glyphs |
| UI_MUTED | 12,14,15 | 0x638f | Neutral helper text and counter |
| UI_LINE | 24,26,26 | 0xc69a | Board edges and neutral borders |
| UI_PAPER / UI_WHITE | 29,30,30 / white | 0xef9e / 0xffff | Surface/HUD backing and empty holes/profile glyphs |
| UI_BLUE / UI_FOCUS / UI_SELECTED | 3,10,24 / 0,19,29 / 25,29,31 | 0x1a98 / 0x04dd / 0xcf5f | Ordinary actions, thinking dots, menu focus/backing |
| UI_NEXT / UI_UNDO / UI_RESTART / UI_RUN | action roles | 0x07ff / 0xf81f / 0xffe0 / 0xf800 | NEXT, UNDO, RESTART, primary action/warning |

Beta.2 used RGB5(25,12,0), 0xcb00, for NORMAL/YELLOW text. Beta.3 replaces that
orange-brown tone with the single 0xac00 gold token, while retaining bright
0xfe00 piece fill. No separate NORMAL brown or second YELLOW trail color remains.
`ui_actor_color()` and `ui_level_color()` resolve to the same token; option arrays
use it directly. One-pixel trail/heads use text gold, never bright piece fill.

Decorative camp tints remain RGB5(28,30,28), (31,26,26), (31,30,22), (24,30,26);
modal shadow remains (12,14,14), and neutral title helper remains (25,28,28).
These are background/shadow roles, not alternative NORMAL/YELLOW text colors.
Action yellow (RESTART) is also a separate action role. No other semantic player
or difficulty magic literal is scattered through rendering functions.

[Palette sheet](screenshots/yellow-palette-sheet.png) shows YELLOW fill, YELLOW
TURN/progress/result, NORMAL option/3P HUD/saved summary/result difficulty and YELLOW trail together. Tests assert exact
RGB565 identity, preserved fill and complete glyph colors. Shared-lane captures
prove both trail colors survive; [visualization](AI_MOVE_VISUALIZATION.md) gives
geometry/lifetime. LCD recognition and contrast remain **HARDWARE TEST REQUIRED**.
