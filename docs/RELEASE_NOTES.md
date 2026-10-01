# DIAMOND v0.1.0-beta.1

First public experimental prerelease for CASIO fx-CG50.

- Korean 73-hole Diamond Game with ten equal pieces, steps and chained jumps.
- Human RED versus AI: two-player GREEN or independent three-player GREEN/YELLOW.
- EASY seeded heuristic play, NORMAL bounded search, HARD larger bounded search;
  alpha-beta for 2P and independent MaxN for 3P.
- Solid pieces/cyan Assist, three colored difficulty faces, AI menu icons,
  red human annotation and black/white RULES keys.
- Assist, one-human-decision Undo, confirmed Restart, Zoom and one-game Resume.
- Crash-safe checksummed A/B storage; old EASY/HARD v1 IDs preserved; NORMAL ID 2.
- System-setting dim/APO support, physical-key brightness restoration, foreground
  cancellation and committed-state checkpoints before MENU/OFF/APO.

See [beta validation](BETA_VALIDATION.md), [AI audit](AI_DIFFICULTY_AUDIT.md),
[power audit](POWER_AUDIT.md) and [memory measurements](MEMORY.md) for evidence.
The package is rebuilt from the exact public source. Release assets are
`DIAMOND.g3a` and `SHA256SUMS.txt`; verify with `shasum -a 256 -c SHA256SUMS.txt`.

**HARDWARE TEST REQUIRED.** Host tests, mocked native APIs and actual renderer
captures establish software behavior, not physical LCD, OS query compatibility,
persistence, MENU/OFF/APO operation or calculator AI response time. All
[28 hardware acceptance checks](HARDWARE_RETEST.md) remain pending. The initial
NORMAL short-response and HARD roughly 1–2-second targets are unmeasured.
If both timer and RTC wake allocation fail, autonomous dim/APO is unavailable
and the add-in displays a warning. This is a beta, not a stable v1.0 release.
