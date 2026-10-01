# Beta memory and package measurements

Baseline is the pre-NORMAL UI build `1f26635`. After is the final beta profile
source built with the same SH GCC 14.1.0 / gint 2.11 SDK and strict `-Os` flags.
Measurements come from the linked ELF, `nm -S`, compiler `.su` files and G3A.

| Measurement | Before | After | Change |
|---|---:|---:|---:|
| `.text` | 42,160 | 42,960 | +800 |
| `.rodata` | 13,256 | 13,372 | +116 |
| `.data` | 80 | 80 | 0 |
| `.bss` | 6,656 | 7,648 | +992 |
| `.gint.bss` | 112 | 112 | 0 |
| AI workspace, including readiness flag | 5,261 | 6,257 | +996 |
| Host AI workspace | 5,273 | 6,273 | +1,000 |
| AI transposition table | 0 | 0 | 0 |
| App controller | 376 | 376 | 0 |
| Largest own stack frame, storage save | 1,344 | 1,344 | 0 |
| Native framebuffers | 1 | 1 | 0 |
| Framebuffer pixel storage | 177,408 | 177,408 | 0 |
| gint framebuffer allocation | 177,504 | 177,504 | 0 |
| G3A bytes | 85,084 | 86,000 | +916 |

The deeper three-player HARD uses the existing shared search, with seven-ply
candidate/ancestor storage in one fixed workspace. The workspace increase is
about 19%; there is no cloned AI, heap allocation or large TT. Each recursive
frame is small; compiler frame sizes do not measure total call-stack high water.
The native framebuffer and runtime allocations are unchanged.

The native package has no personal build-path strings. ELF debug information
and raw build/map files are local-only and excluded from public source/assets.
`tools/memory_report.py` regenerates a local JSON report with every own frame.
The public validation record confirms the exact-source rebuild.

Host process peak measurements are recorded in [BETA_VALIDATION.md](BETA_VALIDATION.md).
The exact public-candidate benchmark measured 1,785,856 bytes maximum RSS
(+16,384) and 1,294,696 bytes peak footprint (unchanged).
The baseline `ai_audit --bench` process measured 1,769,472 bytes maximum RSS and
1,294,696 bytes peak memory footprint under macOS `/usr/bin/time -l`. Process
RSS includes host libraries/runtime and OS page accounting; it is not AI-owned
RAM or calculator heap. Compiler stack reports, process RSS and the linked BSS
cannot establish native total stack/heap high water. Long-play measurement is
**HARDWARE TEST REQUIRED**.
