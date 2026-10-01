# Beta.2 memory and package measurements

Before is beta.1 public `f7e111c9c2a12243275875473fbbc30ecb34d628` / local
`42e5a55`. After uses the same SH GCC 14.1.0 / gint 2.11 SDK and strict `-Os`.
Values come from the linked ELF, `nm -S`, compiler `.su` files and packaged G3A.

| Measurement | Beta.1 | Beta.2 | Change |
|---|---:|---:|---:|
| `.text` | 42,960 | 44,240 | +1,280 |
| `.rodata` | 13,372 | 13,720 | +348 |
| `.data` | 80 | 80 | 0 |
| `.bss` | 7,648 | 7,648 | 0 |
| `.gint.bss` | 112 | 112 | 0 |
| AI workspace, readiness included | 6,257 | 6,257 | 0 |
| Host AI workspace | 6,273 | 6,273 | 0 |
| AI transposition table | 0 | 0 | 0 |
| App controller | 376 | 376 | 0 |
| Largest own frame, storage save | 1,344 | 1,344 | 0 |
| Renderer `dg_render` frame | 76 | 300 | +224 |
| Native cancellation callback frame | 20 | 28 | +8 |
| Native framebuffers | 1 | 1 | 0 |
| Framebuffer pixel storage | 177,408 | 177,408 | 0 |
| gint framebuffer allocation | 177,504 | 177,504 | 0 |
| G3A bytes | 86,000 | 87,628 | +1,628 |

The visual phase fits existing controller padding. The fixed RTC origin and
new code do not increase the final aligned BSS. The measured warning line buffer accounts for the renderer frame increase;
the partial THINKING renderer does not allocate that warning buffer. No AI workspace/profile/budget,
heap allocation, framebuffer count or TT has changed. Largest single-frame
size is not total nested stack high water; gint/OS/libc and runtime heap still
need the hardware long-play measurement.

`tools/memory_report.py` regenerates local JSON with every own compiler frame.
Private-path ELF debug sections, maps and raw logs are excluded from publication.
The G3A package passes path hygiene checks and is rebuilt from the exact public
source. [Beta validation](BETA_VALIDATION.md) records that rebuild. Existing
beta.1 host RSS was 1,785,856 bytes and peak footprint 1,294,696 bytes; neither
is a calculator RAM figure. UI animation uses no host/native heap allocation.
