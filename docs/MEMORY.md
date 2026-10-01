# Beta.3 memory and package measurements

Before is verified beta.2 public `775310a` / local `d49c5d3`. After uses the same
SH GCC 14.1.0, gint 2.11 and strict `-Os`. Linked ELF sections, `nm -S`, compiler
`.su` files and packaged G3A provide these values.

| Measurement | Beta.2 | Beta.3 | Change |
|---|---:|---:|---:|
| `.text` | 44,240 | 46,064 | +1,824 |
| `.rodata` | 13,720 | 13,684 | -36 |
| `.data` | 80 | 80 | +0 |
| `.bss` | 7,648 | 7,888 | +240 |
| `.gint.bss` | 112 | 112 | +0 |
| AI workspace incl. readiness | 6,257 | 6,257 | +0 |
| AI transposition table | 0 | 0 | +0 |
| SH app controller | 376 | 608 | +232 |
| Largest own frame, storage save | 1,344 | 1,344 | 0 |
| Native framebuffers | 1 | 1 | +0 |
| Framebuffer pixels | 177,408 | 177,408 | +0 |
| gint framebuffer allocation | 177,504 | 177,504 | +0 |
| G3A bytes | 87,628 | 89,416 | +1,788 |

New presentation fields total **231 bytes**: 73-byte pre-board, two 77-byte
trails (154), and actor/phase/ticks (4). Compile-time size assertions verify the
compact char-only layouts. Alignment adds one byte to the 232-byte controller
increase; linked BSS rises 240 bytes. Replay reuses the existing 75-byte DgPath,
whose node array has 74 entries. No archive/undo payload grows.

Largest own compiler frame remains 1,344 bytes (storage save). `dg_render`
remains 300 bytes; the board frame is 416 (previously 420); new `ui_trails`
uses 104 bytes. These are single function frames, not total nested stack high
water. AI workspace remains 6,257 SH / 6,273 host bytes with zero TT.
The UI uses no heap allocation, no per-AI malloc/free and one native framebuffer.

`tools/memory_report.py` regenerates local JSON and all own compiler frames.
Private-path ELF/maps/raw logs remain excluded from publication. Package SHA256:
`4096f14bda7a500042162a522ba2dc568e9bc2bf4d6fac00504ba157232a7bf2`.
The exact public source rebuild and uploaded asset re-download must match those
bytes. [Validation](BETA_VALIDATION.md) records software evidence.
Actual gint/OS/libc nested stack, heap high water and calculator frame/search
timing remain **HARDWARE TEST REQUIRED**. Earlier beta.1 host RSS/footprint were
1,785,856 / 1,294,696 bytes; neither is current calculator RAM evidence.
