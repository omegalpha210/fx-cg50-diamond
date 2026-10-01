# DIAMOND

- Korean 73-hole rules: Korea Board Games, then Korean Wikipedia, then documented project decisions. Never add Japanese king/camp rules.
- The engine's move generator is the single source of legality for human input, Assist and AI. CPU moves must pass engine validation.
- Human is RED; 2P CPU is GREEN; 3P uses independent MaxN players.
- One unfinished resume, atomic A/B copies. Checkpoint committed state before MENU/OFF; no cursor/search-node flash writes.
- Gameplay: F1 restart confirmation, F2 one-human-decision undo, F5 overview/zoom. Normal EXIT deselects, then zooms out, then checkpoints to setup; busy cancellation bypasses this hierarchy.
- PLAYER F1 is global RESUME. SETUP has optional matching RESUME, NEW GAME, DIFFICULTY, FIRST/HUMAN, ASSIST last; selectors clamp. EXE/OPEN resumes on RESUME and starts NEW from every other row.
- THINKING uses the existing foreground cancellation hook, 40 RTC ticks per dot; never consume RNG, alter search or reset idle for visual work.
- Label calculator-only validation HARDWARE TEST REQUIRED. Host timing is never calculator timing.
- DIFF EQ, SOKOBAN and NUM GAME directories are read-only references.
- Strict host and SH builds; keep milestones buildable. Publish repositories/releases only when the user explicitly requests them; audit public history and rebuild the exact public source first.
