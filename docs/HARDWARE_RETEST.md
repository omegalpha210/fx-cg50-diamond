# fx-CG50 beta.2 hardware retest

All 32 items are **PENDING — HARDWARE TEST REQUIRED**. Record calculator/OS
version, beta.2 package SHA256, system dim/APO settings and observed results.
Host pixels, transition tests and native API doubles cannot establish hardware behavior.

| # | Check | Acceptance |
|---:|---|---|
| 1 | PLAYER profile and AI | Crisp white head/shoulders in RED; black AI centered in GREEN/YELLOW; balanced weight |
| 2 | PLAYER without save | F1/F2 blank; no SET; F4 RULES and F6 NEXT |
| 3 | Global F1 RESUME | Saved 3P with 2P tile and saved 2P with 3P tile restore original run immediately |
| 4 | 2P setup without matching save | 1 NEW GAME, 2 DIFFICULTY, 3 FIRST, 4 ASSIST |
| 5 | 2P setup with matching save | 1 RESUME, 2 NEW GAME, 3 DIFFICULTY, 4 FIRST, 5 ASSIST |
| 6 | 3P setup without matching save | 1 NEW GAME, 2 DIFFICULTY, 3 HUMAN, 4 ASSIST |
| 7 | 3P setup with matching save | 1 RESUME, 2 NEW GAME, 3 DIFFICULTY, 4 HUMAN, 5 ASSIST |
| 8 | Mismatched setup | Local RESUME absent; first-screen global F1 still available |
| 9 | Row navigation/focus | Default RESUME else NEW; aligned numbering; UP/DOWN focus separate from option outline |
| 10 | Difficulty options | Green EASY, readable gold NORMAL, red HARD, all on one normal-font row; LEFT/RIGHT clamps |
| 11 | 2P FIRST | HUMAN/AI clamp; actual first actor matches; EXE does not cycle |
| 12 | 3P HUMAN | 1ST/2ND/3RD clamp; NEW randomizes only AI colors; restart/resume preserve saved order |
| 13 | ASSIST global preference | Last row OFF/ON; cyan legal holes on/off; preference survives setup EXIT, resume and cold load |
| 14 | EXE/F6 OPEN | RESUME restores saved settings; every other row starts current NEW without cycling or prompting; safe replacement |
| 15 | THINKING actual animation | Text only; 1→2→3→1 dots about 312.5ms apart; fixed origin; no old dot residue or spinner |
| 16 | THINKING board bounds | Overview clear of board/pieces; zoom backing stays compact and readable |
| 17 | Top HUD | TURN : dark prefix; HUMAN red/YELLOW AI gold/GREEN AI green; whole centered mode/level colored; counter clear |
| 18 | Goal progress | No dots; names and fractions colored; only YOU/GREEN in 2P and YOU/YELLOW/GREEN in 3P |
| 19 | Warnings/panels | Long notices wrap visibly in overview gutter; minimal padding in zoom; no oversized opaque area |
| 20 | Selected overview EXIT | First deselects; second checkpoints to setup with matching RESUME retained |
| 21 | Zoom EXIT sequence | Selected: deselect, overview, setup; unselected: overview then setup |
| 22 | Busy EXIT | During all AI profiles and zoom, cancel pending move/RNG, save committed state, go directly to setup |
| 23 | Modal priority/results | Restart EXIT cancels; RULES returns to parent; result EXIT shows frozen board; NEW retains difficulty |
| 24 | Actual piece/cursor contrast | RED/GREEN/YELLOW/white/cyan interiors and thin outlines clear; blue cursor/yellow selection legible in both views |
| 25 | Undo/Restart | All levels/slots: one decision plus replies, repeat undo disabled; restart restores original seed/order/RNG |
| 26 | Native persistence/recovery | Old EASY/HARD IDs, new NORMAL cold load, undo, A/B recovery and finished-run tombstone; save fault retry retains old resume |
| 27 | System dim | Actual OS Backlight Duration 30/60/180s; dots/animation do not reset idle; timer/RTC fallback and allocation warning observed |
| 28 | Brightness restore | Original PWM restored once on physical input, including held keys; same key delivered once |
| 29 | APO | Configured 10/60min idle threshold, including search; cancel/save before OFF; finite save-failure retry |
| 30 | MENU and SHIFT+AC/ON | Save/close/pause/restore before OS handoff; wake/return input barriers; close failure prevents OS entry |
| 31 | Native AI latency | Measure EASY/NORMAL/HARD 2P/3P opening/middle/congestion; dot/cancel responsiveness; HARD about 1–2s is an unverified target |
| 32 | Long play and memory | UI/search/animation persistence stable; storage interruptions recover; native total stack/heap high water measured |

Do not mark a row passed without physical evidence. Forced power interruption,
BFile faults, native timer resources and LCD/PWM behavior need actual observations.
