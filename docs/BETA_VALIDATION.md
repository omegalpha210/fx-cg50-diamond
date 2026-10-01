# Beta validation record

Release source: public-safe `v0.1.0-beta.1` snapshot. This report is finalized
before publication after clean validation in that source tree.

## Release checks

The nine existing CTest cases remain. Added `difficulty` verifies synthetic old
EASY/HARD archives and NORMAL cold load/recovery/result NEW; `publication`
audits the public allowlist. Existing app tests now cover all three difficulties
for cancellation, one-human-turn Undo and Restart across all player/turn slots.
The freeze check permits only difficulty validation and its selector mapping,
while preserving source hashes, topology, moves/paths, EASY/RNG, undo, v1 codec,
transaction/native storage and all 21 rule strings.

The candidate clean build executes `tools/validate_beta.sh`: strict host C11,
full UBSan, native SH strict compile/link, 16 G3A package/hygiene checks, font and
board regeneration checks, and current actual renderer captures. The complete
AI audit also repeats 300 paired 2P games, 144 mixed 3P games, 144 balanced 3P
NORMAL/EASY control games and 32 fixed positions per mode with every legal
root move scored by the host reference. Per-game raw logs stay under `build`;
curated evidence is under [ai](ai/summary.json).

| Check | Exact public candidate result |
|---|---|
| Strict Release CTest | 11/11 PASS; existing nine cases retained |
| Full UBSan CTest | 11/11 PASS, no sanitizer failure |
| Engine | 25,186,841 checks; 5,000 reachable boards; 71,645 chained moves |
| Storage | 10,741 checks plus new difficulty fixture/cold-load/recovery test |
| App | 1,522 checks; all difficulties, player counts and human slots |
| Native API shim | 49,425 checks; actual foreground functions, mocked hardware |
| Power settings | 48 OS-value/fallback combinations and idle/lifecycle matrix |
| Renderer | 227 immutable frames; 1,180,925 bounded rectangles; 37 actual captures |
| SH compile/link | Clean build, all strict warnings enabled, zero warnings |
| G3A | 16 packaging/path-hygiene checks PASS; 86,000 bytes |
| Regeneration/provenance | Board, pinned font, public paths and relative links PASS |
| ASan | **NOT VERIFIED**: Darwin runtime initialization hung before main, including a minimal probe |

G3A SHA256: `6b88ffc3302161f92700d6d3179d2db1503d55df22b0d086e3d9ff57e7794ac3`.
The same bytes were produced by the clean public-source build and local build.
No native debug ELF, map, compiler/SDK binary or raw build log is published.
The exact public candidate repeated all 588 tournament/control games and both
1,200-ply follow-ups. Curated choices, RNG, regret, visited nodes, outcomes and
follow-ups matched the frozen reports exactly after excluding timing fields.

macOS `/usr/bin/time -l build/host/ai_audit --bench` measured maximum RSS
1,785,856 bytes versus baseline 1,769,472 (+16,384); reported peak footprint
was 1,294,696 bytes in both. Host workspace is 6,273 bytes versus 5,273;
[SH sections and frames](MEMORY.md) measure native static storage. Process page
accounting and individual frames do not establish calculator runtime high water.

## Hardware boundary

Actual LCD contrast/colors, OS query values, brightness restoration, dim/APO,
MENU/OFF return, native file persistence and native response time remain
**HARDWARE TEST REQUIRED**. All [28 checks](HARDWARE_RETEST.md) are pending.
Host CPU timings are relative software evidence, not calculator timings.
Diagnostic repeated-state or ply-cap stops exist only in audit tools; the game
rules have no artificial draw or turn limit. The reference shares the evaluator
and selective search, so regret is not a perfect-play proof.

## Publication

Old local history contains private paths/reference screenshots and stays local.
The separate public snapshot includes original code/assets and retained
licenses only. G3A and SHA256SUMS are built in the candidate source tree, then
uploaded as the prerelease's two assets. Download/byte comparison results are
recorded in the published release notes and final delivery report.
