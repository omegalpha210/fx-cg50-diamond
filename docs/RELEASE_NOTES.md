# DIAMOND v0.1.0-beta.2

UI/UX polish prerelease for CASIO fx-CG50, preserving beta.1 game rules, AI
strength/profiles/RNG and v1 A/B storage/power semantics.

- Text-only THINKING dots cycle at 312.5ms through the existing foreground hook;
  fixed small region, no spinner, RNG consumption or internal idle reset.
- Dark TURN : prefix and colored actors, full difficulty-colored centered mode
  label, colored goal names/fractions without dots, content-sized panels/warnings.
- White profile silhouette in RED PLAYER icons; centered black AI retained.
- PLAYER F1 global RESUME works independently of the selected player-count tile.
- NUM GAME-style numbered setup: optional matching RESUME, NEW GAME, DIFFICULTY,
  FIRST/HUMAN and ASSIST last. LEFT/RIGHT clamps; EXE/F6 OPEN resumes only on
  RESUME and starts current NEW from every other row. Legacy SET/SETTINGS removed.
- Normal EXIT deselects → zooms out → checkpoints to setup. Busy CPU EXIT retains
  direct cancel/save/setup; restart/rules/result exits keep precedence.

[UI audit](UI_POLISH_BETA2.md), [validation](BETA_VALIDATION.md),
[acceptance](ACCEPTANCE.md), [captures](screenshots/README.md) and
[memory](MEMORY.md) record exact behavior, pixel/transition tests and source rebuild.
Strict host and full UBSan: 11/11; warning-free SH; 16 package checks;
69 actual renderer captures; 720 EASY goldens and 192 AI choices/stats unchanged.
The package is 87,628 bytes, SHA256
`5c4f9561e0b0bc46dc31ffcb1a7e811c714b044e4c3e5c703debe9c9b3145438`.
Assets: `DIAMOND.g3a`, `SHA256SUMS.txt`. Verify with
`shasum -a 256 -c SHA256SUMS.txt`.

**HARDWARE TEST REQUIRED.** All [32 physical acceptance checks](HARDWARE_RETEST.md)
remain pending: LCD colors/contrast, actual dots and input response, native
persistence, OS dim/APO, MENU/OFF and native AI latency/memory. HARD roughly
1–2 seconds remains an unmeasured target. Host timing and mocked native APIs
are not calculator evidence. ASan remains NOT VERIFIED due to the previously
observed Darwin runtime startup hang. This is an experimental beta prerelease.
