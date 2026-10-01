# DIAMOND for CASIO fx-CG50

<img src="assets/icon-uns.png" width="92" height="64" alt="DIAMOND app icon">

A native Korean 73-hole Diamond Game. Play RED against GREEN in two-player
mode, or against independent GREEN and YELLOW AI players in three-player mode.
Move ten equal pieces into the opposite camp with steps and chained jumps.

[한국어](README_KO.md) · [Download beta](https://github.com/omegalpha210/fx-cg50-diamond/releases/tag/v0.1.0-beta.1) · [Rules](docs/GAME_RULES.md)

![Three-player game, actual shared renderer](docs/screenshots/3p-overview.png)

**Experimental beta — HARDWARE TEST REQUIRED.** Host tests and package checks
pass; calculator display, persistence, power behavior and AI latency still need
the [28 hardware checks](docs/HARDWARE_RETEST.md).

- **2P / 3P:** the human is always RED. Choose the first player or your turn slot.
- **EASY / NORMAL / HARD:** green smile, yellow neutral face, red angry face.
  EASY uses seeded heuristic choices; NORMAL uses a small bounded search; HARD
  uses a larger bounded search. Two-player search uses alpha-beta; three-player search
  uses MaxN with each AI maximizing its own score. [Measured audit](docs/AI_DIFFICULTY_AUDIT.md).
- **ASSIST:** cyan filled holes mark legal destinations. Turn it off for unaided play.
- **UNDO:** take back one human decision and all following AI replies, once.
- **RESTART:** confirm a fresh board with the same seed, difficulty and turn order.
- **ZOOM:** toggle the enlarged board without changing selection or navigation.
- **RESUME:** one unfinished game, with alternating checksummed A/B saves and recovery.
- **Power:** system dim/APO settings, physical-key wake and committed-state checkpoints
  before MENU/OFF/APO. [Implementation and test limits](docs/POWER_AUDIT.md).

![PLAYER](docs/screenshots/player.png)
![Three difficulty faces](docs/screenshots/level-faces.png)

## Install

Download `DIAMOND.g3a` and `SHA256SUMS.txt` from the
[beta release](https://github.com/omegalpha210/fx-cg50-diamond/releases).
Verify the checksum, copy the G3A to the fx-CG50 storage root over USB, safely
disconnect, and launch DIAMOND in the CASIO Main Menu.

```sh
shasum -a 256 -c SHA256SUMS.txt
```

PLAYER defaults to 3P. EXE/F6 opens GAME SETUP. LEFT/RIGHT clamps through
EASY → NORMAL → HARD; DOWN selects FIRST (HUMAN/AI in 2P) or HUMAN
(1ST/2ND/3RD in 3P). EXE/F6 starts. F1 SET changes global ASSIST;
F2 RESUME appears for an unfinished saved game. Resume bypasses setup and
preserves the saved difficulty, turn order, RNG and undo snapshot.

## Controls

| Key | Action |
|---|---|
| Arrows | Move cursor in overview/zoom; change menu choices |
| EXE / F6 | Select your RED piece, then commit a legal destination |
| EXIT | Cancel selection; otherwise checkpoint and return to setup |
| F1 | Confirm restart |
| F2 | Undo one human decision and all following AI replies |
| F4 | Read scrollable rules |
| F5 | Toggle overview / zoom |
| MENU | Checkpoint and open the real CASIO Main Menu |
| SHIFT + AC/ON | Checkpoint and power off through gint |

ASSIST ON includes every legal final jump landing. The displayed path is one
shortest representative route. Retracing is allowed during a jump chain; a turn
ending on its starting hole is excluded. Steps and jumps cannot mix in a turn.
No capture, king pieces or Japanese camp restrictions are used. Goal pieces may
leave until someone wins. The first player with all ten pieces in the goal wins.

`DGSTATEA.dat` and `DGSTATEB.dat` are generations of one archive. Search and
cursor redraws do not write flash. New games replace the resume after verified
storage success. Old EASY/HARD v1 saves retain IDs 0/1; NORMAL uses ID 2 in the
same format. [Save specification](docs/STORAGE_FORMAT.md).

## Build and validate

Requires CMake, a C11 compiler and Python 3 for host tests. The native build
requires fxSDK, gint 2.11 and the SH cross compiler. Set `DIAMOND_SDK_ROOT` to
your SDK prefix layout; its default is `$HOME/.local/diffeq-sdk`.

```sh
bash tools/test.sh
bash tools/build.sh
cmake -S . -B build/ubsan -DDG_HOST=ON -DDG_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ubsan -j8
ctest --test-dir build/ubsan --output-on-failure
```

The native build produces `dist/DIAMOND.g3a` and `dist/SHA256SUMS.txt`.
Pillow is needed only for font/icon regeneration and screenshot conversion;
committed source assets build without it. [Development](docs/DEVELOPMENT.md),
[validation](docs/BETA_VALIDATION.md), [memory](docs/MEMORY.md),
[actual renderer captures](docs/screenshots/README.md), and
[publication provenance](docs/PUBLICATION.md) record the beta's evidence.
Host milliseconds are not fx-CG50 timings; this AI is bounded, selective search.

## Rules and license

Rules are independently summarized from
[Korea Board Games](https://www.koreaboardgames.com/magazine/menuDetail?boardCd=contents&postNo=314)
and [Korean Wikipedia](https://ko.wikipedia.org/wiki/다이아몬드_게임).
[Source decisions](docs/RULE_SOURCES.md) and [geometry](docs/BOARD_GEOMETRY.md)
explain the conventional 73-hole star: adjacent ten-hole camps share six boundary
corners, leaving 19 holes outside their union.

Original game code, generated board/icons and own renderer captures use the
[MIT license](LICENSE). Adapted helpers, the gint font and linked runtimes retain
[their own notices](THIRD_PARTY_NOTICES.md). No source article, board photo,
Sokoban map or private reference screenshot is distributed. CASIO compatibility
does not imply affiliation or endorsement.
