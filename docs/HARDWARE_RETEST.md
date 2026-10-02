# fx-CG50 beta.4 hardware retest

The original 46 items and additions below are **PENDING — HARDWARE TEST REQUIRED**. Record calculator/OS
version, beta.4 package SHA256, system dim/APO settings and observed results.
Host pixels, transition tests and native API doubles cannot establish hardware behavior.

| # | Check | Acceptance |
|---:|---|---|
| 1 | PLAYER profile and AI | Crisp white head/shoulders in RED; black AI centered in GREEN/YELLOW; balanced weight |
| 2 | PLAYER without save | F1/F2 blank; no SET; F4 RULES and F6 NEXT |
| 3 | Global F1 RESUME | Saved 3P with 2P tile and saved 2P with 3P tile restore original run immediately |
| 4 | 2P setup without matching save | 1 NEW GAME, 2 DIFFICULTY, 3 FIRST, 4 ASSIST |
| 5 | 2P setup with matching save | 1 RESUME, 2 NEW GAME, 3 DIFFICULTY, 4 FIRST, 5 ASSIST |
| 6 | 3P setup without matching save | 1 NEW GAME, 2 DIFFICULTY, 3 YOU, 4 ASSIST |
| 7 | 3P setup with matching save | 1 RESUME, 2 NEW GAME, 3 DIFFICULTY, 4 YOU, 5 ASSIST |
| 8 | Mismatched setup | Local RESUME absent; first-screen global F1 still available |
| 9 | Row navigation/focus | Default RESUME else NEW; aligned numbering; UP/DOWN focus separate from option outline |
| 10 | Difficulty options | Green EASY, readable gold NORMAL, red HARD, all on one normal-font row; LEFT/RIGHT clamps |
| 11 | 2P FIRST | YOU/AI clamp; actual first actor matches; EXE does not cycle |
| 12 | 3P YOU | 1ST/2ND/3RD clamp; NEW randomizes only AI colors; restart/resume preserve saved order |
| 13 | ASSIST global preference | Last row OFF/ON; cyan legal holes on/off; preference survives setup EXIT, resume and cold load |
| 14 | EXE/F6 OPEN | RESUME restores saved settings; every other row starts current NEW without cycling or prompting; safe replacement |
| 15 | THINKING actual animation | Text only; 1→2→3→1 dots about 312.5ms apart; fixed origin; no old dot residue or spinner |
| 16 | THINKING board bounds | Overview clear of board/pieces; zoom backing stays compact and readable |
| 17 | Top HUD | TURN : dark prefix; YOU red/YELLOW AI gold/GREEN AI green; whole centered mode/level colored; counter clear |
| 18 | Goal progress | No dots; names and fractions colored; only YOU/GREEN in 2P and YOU/YELLOW/GREEN in 3P |
| 19 | Warnings/panels | Long notices wrap visibly in overview gutter; minimal padding in zoom; no oversized opaque area |
| 20 | Selected overview EXIT | First deselects; second checkpoints to setup with matching RESUME retained |
| 21 | Zoom EXIT sequence | Selected: deselect, overview, setup; unselected: overview then setup |
| 22 | Busy EXIT | During all AI profiles and zoom, cancel search without committing its move/RNG, or skip already committed replay; save final committed state, go directly to setup |
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
| 33 | Hop animation speed | Measure actual STEP/JUMP at 100–150ms per segment; nominal 15/128s, no teleport |
| 34 | Long-chain readability | Every hop follows the final engine path in sequence, including turns |
| 35 | Arrow direction | All six directions and both views point from source toward destination; one head per hop |
| 36 | YELLOW trail visibility | Gold 1px path and heads remain readable on LCD alongside bright YELLOW pieces |
| 37 | GREEN trail visibility | Green path/heads remain readable, distinguishable from edges and pieces |
| 38 | Shared-edge dual lanes | Same and opposite directions retain both colors on canonical ±1px lanes |
| 39 | Trails during YOU turn | 2P retains GREEN; 3P retains actual preceding YELLOW and GREEN paths |
| 40 | Clear after YOU move | Only legal YOU commit clears both; invalid move/cursor/select/deselect/zoom preserve |
| 41 | ZOOM trails | Clipping, trim, heads and lanes remain correct throughout pan |
| 42 | Assist plus trails | Cyan destinations and cursor/selection remain clear above both AI trails |
| 43 | CPU latency plus animation | Measure search time separately from final replay in all six profiles; no strength/timing claim from host |
| 44 | MENU/OFF during replay | Skip immediately; persist final move/RNG/turn, never pre-board or partial hop; terminal replay preserves tombstone |
| 45 | YELLOW semantic palette | NORMAL options/HUD, YELLOW HUD/progress and trails share 0xac00; fill alone is 0xfe00 |
| 46 | YOU terminology | PLAYER, 2P FIRST, 3P YOU slot, TURN, progress, rules/help and results contain no visible HUMAN |

Do not mark a row passed without physical evidence. Forced power interruption,
BFile faults, native timer resources and LCD/PWM behavior need actual observations.

## USB and visual candidate — additional gate

All cases below are **HARDWARE TEST REQUIRED**, without recorded physical results.
For every USB case, record VBUS detection availability, Main Menu return, actual
Select Connection Mode appearance and recurrence of the original white flash
separately. Repeat successful cases; one success is insufficient to claim a fix.

- Insert while idle, in setup/modal, during gameplay and during each AI profile.
- Insert during a committed multi-hop replay and confirm saved final board/RNG.
- Insert while dimmed; combine with MENU and SHIFT+AC/ON; verify one transition.
- Keep cable attached, unplug and reinsert; verify one request per observed edge.
- Select USB Flash and verify save files, then test ScreenRecv, ScreenRecv(XP)
  and Projector availability. Record if reconnection was actually necessary.
- Check normal/selected Main Menu icon top geometry and label gap after uniform 92% scaling (17px bottom / 1px top white margin).
- Check bright yellow/light green, 2px overview / 3px zoom, larger arrows in all
  directions, ±3px shared lanes, and cyan Assist distinction on the LCD.

The owner explicitly authorized an experimental beta.4 binary with physical tests pending. USB is not claimed fixed. See [audit and detector limits](USB_LIFECYCLE_AUDIT.md).

## Beta.4 rules, saves and endgame additions

All remain **PENDING — HARDWARE TEST REQUIRED**.

- NEW 2P/3P: reject opponent-only camp and all four boundary-row landings;
  show red OPPONENT CAMP with Assist ON/OFF. Own shared HOME/GOAL corners work.
- Resume old EASY/HARD/NORMAL V1 archives, including formerly unrestricted camp
  positions; confirm LEGACY RULES, unchanged board/order/difficulty/RNG/Undo.
- RESTART retains V1; NEW switches to V2. Verify mixed V1/V2 A/B recovery without
  modifying the newest good copy during a failed save.
- Reach 7/8/9 goals as every active color at EASY/NORMAL/HARD. Confirm an immediate
  last-hole win is taken, including GREEN after an opponent vacates its goal.
- Observe extended HARD endgame search latency and foreground EXIT/MENU/OFF/APO/
  USB cancellation. Record native time separately from all host CPU timings.
- Long sessions: watch goal shuffling, legitimate rearrangements, repeated moves,
  stack/heap high water and input responsiveness. No automatic turn cap applies.
- Confirm top geometry is visible and the OS's actual DIAMOND label no longer
  touches normal/selected artwork. Compare the 92% icon on the real Main Menu.
