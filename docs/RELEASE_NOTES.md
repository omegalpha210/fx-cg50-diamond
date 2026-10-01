# DIAMOND v0.1.0-beta.3

AI move presentation prerelease for CASIO fx-CG50. Game rules, AI policies/
strength/order/RNG and v1 A/B storage/power semantics are preserved.

- Final chosen AI move commits once through the engine, then replays its actual
  representative path from a 73-byte visual pre-board. Nominal 117.1875ms per
  hop, four intermediate positions; search shows only existing THINKING dots.
- 2P retains GREEN's last directional trail; 3P retains both preceding AI paths
  until your next legal move. Invalid input/cursor/selection/zoom preserve trails.
  Trimmed one-pixel lines/chevrons protect holes; shared edges use canonical ±1px
  lanes, including opposite movement. No trail is saved or stored in Undo.
- MENU/OFF/EXIT skip replay and preserve final committed state. Winning moves
  checkpoint the inactive tombstone before replay and defer RESULT until its end.
- All visible HUMAN labels are YOU, including FIRST/slot, saved setup, TURN and
  help. Internal human/save fields retain their names/IDs.
- NORMAL and YELLOW actor text/progress/trail share 0xac00; bright YELLOW fill
  remains 0xfe00. [Palette](UI_COLOR_AUDIT.md), [visual behavior](AI_MOVE_VISUALIZATION.md).

Strict Release/full UBSan: 12/12; SH warnings zero; all 16 package checks;
131 actual renderer captures, including a 26-frame five-hop move; 720 EASY goldens,
192 fixed choices/stats/RNG and 192 chosen paths identical. Forty-eight smoke
matches and the still-capped follow-up match beta.2, illegal/crashes zero.
No game/AI/engine/storage/power implementation/interface diff. [Validation](BETA_VALIDATION.md),
[acceptance](ACCEPTANCE.md), [captures](screenshots/README.md), [memory](MEMORY.md).

G3A: 89,416 bytes. SHA256:
`4096f14bda7a500042162a522ba2dc568e9bc2bf4d6fac00504ba157232a7bf2`.
Assets: DIAMOND.g3a and SHA256SUMS.txt; verify with `shasum -a 256 -c SHA256SUMS.txt`.
The exact audited public source rebuild matches local artifacts and captures.
Post-upload download/digest verification is recorded in the GitHub release.

**HARDWARE TEST REQUIRED.** All [46 exact physical checks](HARDWARE_RETEST.md),
including the original 32 and 14 new animation/trail/color/text items, remain
pending. Actual hop speed/frames, LCD/Assist contrast, system keys during replay,
BFile persistence, dim/APO, native AI latency and total stack/heap are unmeasured.
Host seconds are not calculator seconds. ASan remains NOT VERIFIED due to the
previous Darwin runtime initialization hang. This is an experimental prerelease.
