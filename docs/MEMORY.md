# Beta.4 memory and package measurements

Before is the pre-task local integration with experimental USB and bright trails,
not the older public beta.3 binary. Both use SH GCC 14.1.0, gint 2.11 and strict
`-Os`. Values come from the ELF, `nm -S`, compiler `.su` and final packaged G3A.
[Machine-readable summary](ai/beta4/memory.json).

| Measurement | Before | Beta.4 | Change |
|---|---:|---:|---:|
| .text | 47,120 | 51,024 | +3,904 |
| .rodata | 13,684 | 13,976 | +292 |
| .data | 80 | 80 | +0 |
| .bss | 7,888 | 11,632 | +3,744 |
| .gint.bss | 112 | 112 | +0 |
| AI workspace | 6,257 | 9,895 | +3,638 |
| Controller | 608 | 704 | +96 |
| Largest own stack frame | 1,344 | 1,732 | +388 |
| Native framebuffers | 1 | 1 | +0 |
| Framebuffer pixels | 177,408 | 177,408 | +0 |
| gint framebuffer allocation | 177,504 | 177,504 | +0 |
| G3A bytes | 90,472 | 94,668 | +4,196 |

AI workspace now includes the allowed-graph goal-distance cache and nine-ply
candidate/ancestor capacity. The controller includes twelve 8-byte recent-turn
records (96 bytes); revision/history counters fit its existing alignment.
There is no heap allocation per move, transposition table or extra framebuffer.
The visual pre-board remains 73 bytes and trails remain 2×77 bytes. Encoded
archives remain at most 208 bytes under the 256-byte decoder limit.

Largest own frame: **1732 bytes**, storage transaction.
Endgame matching is 400 bytes, maxN 104 per recursive call, alpha-beta 60,
root chooser 112, move generator 284 and reachability BFS 152. These individual
frames are not total nested stack/heap high water; gint/OS/libc stacks and native
long-play resource use still require measurement. The strict SH build retains
`-Wframe-larger-than=2048` and produces zero warnings.

Package: **94,668 bytes**. SHA256:
`2db3242112391659e24901259f1b9c6e8136ec97867dd49ecb449949bb81908a`.
Exact public-source rebuilding and re-downloaded release-asset matching are
recorded in [publication](PUBLICATION.md). Display, search/USB latency and actual
stack/heap high water remain **HARDWARE TEST REQUIRED**.
