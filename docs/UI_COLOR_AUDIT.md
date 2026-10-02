# Semantic color audit — local beta.4 candidate

The common renderer's central `src/ui/draw.h` palette supplies both host and SH.
RGB5 below means the project's `DG_RGB(r,g,b)` convention (green is shifted by
six bits into RGB565). Semantic text colors are separate from fill only where
needed; RED/GREEN retain their existing values.

| Role/token | RGB5 | RGB565 | Actual uses |
|---|---|---|---|
| PIECE_RED | 28,3,3 | 0xe0c3 | RED pieces and menu profile fill |
| UI_RED_TEXT | alias PIECE_RED | 0xe0c3 | YOU label, TURN YOU, YOU progress/result, HARD text |
| PIECE_GREEN | 0,20,8 | 0x0508 | GREEN pieces and AI icons |
| UI_GREEN_TEXT | alias PIECE_GREEN | 0x0508 | GREEN TURN/progress/result, EASY options and mode label |
| PIECE_YELLOW | 31,24,0 | 0xfe00 | YELLOW pieces/icons; existing yellow selection ring |
| UI_YELLOW_TEXT | 21,16,0 | 0xac00 | NORMAL option/outline/underline, saved summary, result difficulty and full mode label; YELLOW TURN/progress/result |
| TRAIL_YELLOW | 30,23,0 | 0xf5c0 | Bright YELLOW trail and filled arrow |
| TRAIL_GREEN | 10,25,4 | 0x5644 | Light GREEN trail and filled arrow |
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

Text retains beta.3's 0xac00 YELLOW/NORMAL gold; piece fills are unchanged.
Trails now have independent tokens: old YELLOW 0xac00 → 0xf5c0 and old GREEN
0x0508 → 0x5644. Cyan Assist remains 0x05da. Arrows use exactly their trail's
color. The brighter yellow is close to the 0xfe00 piece fill; light green has
more red and much less blue than cyan Assist.

Brighter colors have lower luminance contrast than the old dark text colors:
about 1.57:1 yellow and 1.91:1 green against paper using standard RGB565 decoding.
This is not a text-accessibility pass. The 2px/3px strokes, larger heads and
separate lanes provide additional visible area. [Measured color-only ratios](screenshots/trail-contrast.json),
[overview backgrounds](screenshots/trail-contrast-overview.png),
[zoom backgrounds](screenshots/trail-contrast-zoom.png) and
[Assist with trails](screenshots/trails-assist-zoom.png) make the tradeoff reviewable.
The host capture pipeline retains its existing RGB5 green expansion; these ratios
use the actual six-bit green field of the RGB565 constants.

Decorative camp tints remain RGB5(28,30,28), (31,26,26), (31,30,22), (24,30,26);
modal shadow remains (12,14,14), and neutral title helper remains (25,28,28).
These are background/shadow roles, not alternative NORMAL/YELLOW text colors.
Action yellow (RESTART) is also a separate action role. No other semantic player
or difficulty magic literal is scattered through rendering functions.

[Palette sheet](screenshots/yellow-palette-sheet.png) shows the separate YELLOW fill/trail and YELLOW
TURN/progress/result, NORMAL option/3P HUD/saved summary/result difficulty and YELLOW trail together. Tests assert exact
RGB565 token values, preserved fill and complete glyph colors. Shared-lane captures
prove both trail colors survive; [visualization](AI_MOVE_VISUALIZATION.md) gives
geometry/lifetime. LCD recognition and contrast remain **HARDWARE TEST REQUIRED**.
