# Third-party notices

Original game engine, AI, UI and mathematical icon geometry are MIT licensed
under [LICENSE](LICENSE). The UI now uses the existing product family's gint
font, with its separate permission below. No ODE code, Sokoban game maps,
NUM GAME registry or downloaded board picture is bundled in the add-in.

| Reference or dependency | Use |
|---|---|
| NUM GAME (`825b5561914a198629a0490fa229c956c3abdfc6`) | Read-only reference for A/B transaction philosophy, foreground lifecycle, OS scalar power queries and input barriers. Small syscall assembly is adapted; MIT notice retained in `docs/third_party/NUMGAME-LICENSE.txt`. |
| SOKOBAN (`2492545230d49ccc3284bc48edb3ee223111f88f`) | Read-only reference for restart/modal/selector/board UI and native infrastructure. Its font table is reused; proportional text/atlas conversion helpers are adapted under the retained MIT notice. No game map source copied. |
| DIFF EQ (`a6c9659342e2ab7661072cf5916a4ba6ee81e96a`) | Read-only reference for fxSDK, normal typography, form/focus and exact semantic colors. The atlas extraction helper originated here; MIT notice retained. No equations or numerical engine copied. |
| [gint](https://git.planet-casio.com/Lephenixnoir/gint) 2.11, `badbd0fd2bd8ac796fd55d49b93691741bd8a139` | Linked native runtime and proportional font8x9 atlas. Permissive author permission retained in `docs/third_party/gint-README.md`; permits use, sharing, modification and sharing changes. |
| [fxSDK](https://git.planet-casio.com/Lephenixnoir/fxsdk) | External MIT build/packaging tools and G3A format specification. Notice retained. |
| [FxLibc](https://git.planet-casio.com/Vhex-Kernel-Core/fxlibc) | Linked C library, CC0; separate MIT Grisu notice retained. |
| OpenLibm SH port | SDK-linked dependency; combined upstream notices retained. The game and AI use integer arithmetic. |
| GCC runtime | Linked integer support; GCC Runtime Library Exception 3.1 and GPLv3 retained. |

CMake, Python, Pillow and compiler executables are external build dependencies.
The three project directories were only read. Rule-source prose and photographs
are referenced by links in [RULE_SOURCES.md](docs/RULE_SOURCES.md), not copied as
assets. CASIO compatibility does not imply affiliation or endorsement.

Font provenance: `assets/font/font8x9.png` is the exact locally reused gint atlas,
SHA256 `0e7f56f30e021e16053360b972adc0b020c7d8a2d545fbd883bc8e7d870413c8`.
The PNG is a source/regeneration input. Only its 1,140-byte generated glyph table
is compiled into the common UI renderer. `src/ui/font_data.h` reuses SOKOBAN's
table; `tools/font_data.py` and `src/ui/draw.c` adapt its converter/text helper.
Private reference screenshots and Sokoban maps are excluded from this public
snapshot. [The license audit](docs/LICENSE_AUDIT.md) records inspected provenance
and retained terms. Separately identified dependencies retain their own rights;
this project license does not relicense reference content. No online or
proprietary OS font was obtained.
