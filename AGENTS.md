# DIAMOND

- Korean 73-hole rules: owner-supplied physical Korean manual, Korea Board Games, Korean Wikipedia, then documented project interpretations. Never add Japanese king/camp rules. V2 landing policy: own HOME/GOAL membership first, otherwise active opponent HOME/GOAL forbidden; keep all shared physical nodes.
- NEW uses V2. Legacy V1 resumes and RESTART retain V1; save revision explicitly. AI history is transient, bounded to 12 committed turns, and clears on NEW/RESTART/UNDO/RESUME. Immediate legal wins precede search and RNG.
- The engine's move generator is the single source of legality for human input, Assist and AI. CPU moves must pass engine validation.
- Human is RED; 2P CPU is GREEN; 3P uses independent MaxN players.
- One unfinished resume, atomic A/B copies. Checkpoint committed state before MENU/OFF; no cursor/search-node flash writes.
- Gameplay: F1 restart confirmation, F2 one-human-decision undo, F5 overview/zoom. Normal EXIT deselects, then zooms out, then checkpoints to setup; busy cancellation bypasses this hierarchy.
- PLAYER F1 is global RESUME. SETUP has optional matching RESUME, NEW GAME, DIFFICULTY, FIRST/YOU, ASSIST last; selectors clamp. EXE/OPEN resumes on RESUME and starts NEW from every other row.
- THINKING uses the existing foreground cancellation hook, 40 RTC ticks per dot; never consume RNG, alter search or reset idle for visual work.
- Label calculator-only validation HARDWARE TEST REQUIRED. Host timing is never calculator timing.
- DIFF EQ, SOKOBAN and NUM GAME directories are read-only references.
- Strict host and SH builds; keep milestones buildable. Publish repositories/releases only when the user explicitly requests them; audit public history and rebuild the exact public source first.

- Final CPU moves commit once through the engine before visual replay; animation uses a 73-byte pre-board and the existing representative path. System requests skip replay and checkpoint final state; show a winning result only after replay.
- Keep one transient trail per AI until a legal YOU commit; clear on NEW, confirmed RESTART, successful UNDO and RESUME. Never serialize trails. Use YOU in visible text. YELLOW/NORMAL share their text token; AI trails use separate TRAIL_YELLOW/TRAIL_GREEN colors.
