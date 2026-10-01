# Beta.3 validation record

Baseline public source `775310a178da07b275af3ecf0817339cbbae19d2`, local `d49c5d3`,
release v0.1.0-beta.2. Before edits, strict CTest, 720 EASY goldens, 192 fixed
choices/paths, engine golden and 48 representative smoke matches were recorded.
The capped smoke case's follow-up is additionally reproduced with the exact
published beta.2 executable. The current audited successor repeats checks through
`tools/validate_beta.sh` from a clean exact public candidate.

| Check | Result |
|---|---|
| Strict Release CTest | 12/12 PASS, original 11 retained plus visuals |
| Full UBSan CTest | 12/12 PASS, no sanitizer finding |
| Engine | 25,186,841 checks; 5,000 reachable boards; 71,645 chained moves |
| Storage | 10,741 checks; old v1 and NORMAL compatibility retained |
| App | 6,904 checks, including atomic replay/system/winning tombstones |
| Actual native API shim | 316,978 checks; search dots, replay clock/skip/APO, input/power/OS order |
| Existing renderer geometry | 654 immutable frames, 2,107,816 bounded rectangles |
| New visual geometry/lifetime | 3,924 normalized segment cases; 29,256 safe arrow pixels; 48 dual-lane cases; 61 immutable visual frames |
| AI freeze | 15 byte-identical files; 720 EASY goldens; 192 selections/RNG/nodes/depth/beam and 192 paths identical |
| Selfplay regression | 12 paired 2P + 18 mixed 3P + 18 control; 47 finished, one capped; 1200-ply follow-up still capped exactly as beta.2; illegal=0/crashes=0 |
| Captures | 131 current common-renderer frames, including 26 actual five-hop animation frames |
| SH | Strict compile/link, zero warnings |
| G3A | 16 package/path checks PASS; 89,416 bytes |
| Board/font/public policy | Generated data, pinned font, allowlist/path/link checks PASS |
| ASan | NOT VERIFIED: earlier Darwin runtime initialization hung before main even in a minimal probe |

G3A SHA256: `4096f14bda7a500042162a522ba2dc568e9bc2bf4d6fac00504ba157232a7bf2`.
[Memory](MEMORY.md) records before/after sections, frame/workspace and 231-byte
new visual state. No game/AI/engine/storage/power source/interface changes,
strength tuning, save version change, new draw rule or second framebuffer.
The native wake cadence changes only foreground polling; power thresholds and
physical-input-only idle semantics are unchanged.

[Untimed fixtures](../tests/fixtures/README.md) and
[regression fingerprints](ai/ui-beta3-regression.json) make the current smoke
comparison reproducible; only host timing is excluded. The full 588-match beta.2
study remains [historical evidence](ai/ui-beta2-regression.json), not a claim that
beta.3 reran that entire study. Existing bounded/selective AI limitations remain.

The exact public candidate repeats strict Release/UBSan, fixed-choice/path
freeze, smoke games, SH/package and renderer capture generation. Local/candidate
G3A, checksum file and captures compare byte-identically. Uploaded G3A and
SHA256SUMS are then re-downloaded, byte-compared, checksum-verified and checked
against both GitHub SHA256 digest fields. Public history and original tags stay
on their existing ancestry; private local development history is excluded.
Raw build/download/selfplay logs stay local under build.

All [46 physical checks](HARDWARE_RETEST.md) remain PENDING — HARDWARE TEST REQUIRED.
Host pixels/API doubles do not establish LCD contrast, actual hop/frame/search
timing, physical input, BFile/OS handoff, dim/APO or total stack/heap high water.
