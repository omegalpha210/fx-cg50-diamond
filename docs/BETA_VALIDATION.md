# Beta.2 validation record

Release source is the public-safe `v0.1.0-beta.2` successor of beta.1. The local
private development history stays separate; public ancestry is preserved.
The exact clean candidate is verified with `tools/validate_beta.sh` and the
fixed-choice/selfplay regression described below before publication.

All original nine CTest cases remain, alongside existing `difficulty` and
`publication`. The freeze guard now pins the complete beta.1 engine/game/AI/
storage/power source/interface files and both engine/AI tests: 15 hashes.
Engine output and every field except host timing in 192 fixed choices must
match beta.1 exactly. The 720 EASY goldens are preserved in the frozen AI test.

| Check | Result |
|---|---|
| Strict Release CTest | 11/11 PASS; original cases retained |
| Full UBSan CTest | 11/11 PASS, no sanitizer failure |
| Engine | 25,186,841 checks; 5,000 reachable boards; 71,645 chained moves |
| Storage | 10,741 checks plus old v1 and NORMAL compatibility suite |
| App | 6,228 checks: original lifecycle/Undo/Restart/results plus row/resume/EXIT matrix |
| Native API shim | 239,756 checks: actual key/barrier/cancel/system/idle and clock-stepped dots |
| Power settings | All 48 OS-value/fallback combinations retained |
| Renderer geometry | 642 immutable frames; 2,058,781 bounded rectangles |
| Captures | 69 actual current common-renderer frames plus own comparison sheets |
| AI freeze | 192/192 choices/RNG/nodes/depth/beam identical; 720 EASY goldens PASS |
| SH compile/link | Strict warnings enabled, zero warnings |
| G3A | 16 package/path checks PASS; 87,628 bytes |
| Board/font/public audit | Generated board, pinned font, paths, links and allowlist PASS |
| ASan | NOT VERIFIED: prior Darwin runtime initialization hung before main, including minimal probe |

G3A SHA256: `5c4f9561e0b0bc46dc31ffcb1a7e811c714b044e4c3e5c703debe9c9b3145438`.
The exact public candidate and local source produce identical bytes. Compiler
sections/frames/workspace are recorded in [MEMORY.md](MEMORY.md).

The same beta.1 selfplay commands run 300 paired 2P games, 144 mixed 3P games
and 144 balanced NORMAL/EASY controls. Their per-game outcomes, turns, goals,
nodes, RNG-seed/seat configurations and cycle/cap flags matched all 588 prior
records exactly after excluding host timing. Both follow-ups (five cases) at
1,200 plies also matched exactly, including the diagnostic-only cycle/cap policy.
[Untimed regression fingerprints](ai/ui-beta2-regression.json) record counts
and semantic SHA256 values for every group. The existing quality/reference
regret study remains historical beta.1 evidence for the unchanged AI;
[AI_DIFFICULTY_AUDIT.md](AI_DIFFICULTY_AUDIT.md) records its limitations. The
reference shares the evaluator and selective search, and is not a perfect-play oracle.
Raw candidate build, selfplay and download logs stay under `build/`.

Native API doubles and host pixels do not verify actual LCD, OS settings,
brightness, dim/APO, MENU/OFF, BFile persistence, physical input, AI latency or
runtime total stack/heap. All [32 hardware checks](HARDWARE_RETEST.md) remain
PENDING — HARDWARE TEST REQUIRED. Host seconds are not calculator timings.

The two prerelease assets are `DIAMOND.g3a` and `SHA256SUMS.txt`. After upload,
both are downloaded again; byte comparison, checksum-file verification and
GitHub asset SHA256 digests are recorded in the published release notes and
final delivery report. No native debug ELF, SDK binary or raw build log is public.
