# Rule sources and decisions

Read directly on 2026-10-01. Precedence for Korean play: Korea Board Games → Korean Wikipedia → explicit project decisions. Japanese Wikipedia is used only to identify rules that must not be imported. The sources below describe variants; this project does not present its implementation choices as a universal official Korean rulebook.

## SOURCE-SUPPORTED

1. [Korea Board Games, “고전 게임의 발견: 다이아몬드 게임”](https://www.koreaboardgames.com/magazine/menuDetail?boardCd=contents&postNo=314), Park Ji-won, 2017-08-25.

   The article's opening rules explain ten pieces per player, moving one piece per turn, a one-space move or a jump over another piece, further jumps when available, no capture, and ending the jumping sequence when desired. Victory means sending all ten pieces to the ten-space camp opposite the start. The “다이아몬드 게임” section distinguishes the widespread 73-space, ten-piece, at-most-three-player version from 121-space variants. Its description of stronger Japanese pieces is a variant comparison, not a Korean rule to implement.

2. [Korean Wikipedia, “다이아몬드 게임”](https://ko.wikipedia.org/wiki/다이아몬드_게임), read revision [41101678](https://ko.wikipedia.org/w/index.php?title=다이아몬드_게임&oldid=41101678).

   Supports two or three players, 120° separation of two-player starting camps, and choosing the first player/turn direction by agreement. Its age-based color allocation is not used for a game with one human and CPUs.

### Geometry evidence and authorized correction

The Korean article uses [Dmthoth's “Diamond Game.svg” diagram](https://commons.wikimedia.org/wiki/File:Diamond_Game.svg), uploaded 2011-02-06. It was inspected as a reference; its artwork is not copied into this project's assets. The diagram shows the conventional 73-point triangular-lattice star: horizontal row counts `1,2,3,10,9,8,7,8,9,10,3,2,1`. Its external arms have six points each and its central hexagon has 37. A four-row ten-point camp includes four hexagon boundary points; neighboring camps share six corner points in total.

Consequently, this source does **not** support a disjoint `6 × 10 + 13` partition. The user explicitly accepted the conventional 73-point star and ten-piece camps with shared boundaries on 2026-10-01. That correction replaces the original disjoint-region requirement. The implemented partition is 37 central-hexagon points plus six external six-point arms; ten-point camp membership is stored separately and permits shared corners. See [BOARD_GEOMETRY.md](BOARD_GEOMETRY.md) for the coordinate construction and proof. The article text alone provides no coordinate set or adjacency table.

## PROJECT DECISION

- One human, always RED. Two-player CPU is always GREEN. Three-player CPUs use YELLOW and GREEN.
- RED starts below; YELLOW upper left; GREEN upper right. Goals are opposite their respective homes.
- The human chooses the first/turn slot. New three-player games randomize only the remaining CPU slots; resume and restart preserve the order.
- All pieces have the same abilities. A jump crosses one adjacent occupied point and lands on the next empty point in that straight lattice direction; either player's piece can be crossed. Direction can change between jumps. STEP and JUMP cannot be mixed in one turn.
- Any arm can be entered, including unused and opposing camps. Goal pieces can leave until victory. This unrestricted policy is explicitly a project choice.
- First player with all ten pieces in their goal wins immediately, including in three-player play. There are no later rankings, captures, forced jumps, invented score, or game turn limit.
- Jump-state visitation prevents search cycles; it does not create a rule forbidding route retracing. At a repeated landing, every nonmoving piece is unchanged and the moving piece occupies the same point, so the state and future destinations are identical.
- On 2026-10-01 the user explicitly chose to allow route retracing during a chain but exclude a turn ending back at its source. Thus `(from,to)` must have distinct endpoints. This is a project decision, not a claim that Korean sources forbid retracing; it preserves the requested source-empty move invariant.
- Assist, navigation, zoom, save/resume, undo, restart, AI and all software controls are project features.

## NOT USED / JAPANESE VARIANT

3. [Japanese Wikipedia, “ダイヤモンドゲーム”](https://ja.wikipedia.org/wiki/ダイヤモンドゲーム), read revision [107230637](https://ja.wikipedia.org/w/index.php?title=ダイヤモンドゲーム&oldid=107230637).

   Describes a common Japanese arrangement with one king and fourteen ordinary pieces, special long king jumps, ordinary pieces unable to jump kings, restrictions on opposing camps, no route retracing, and a color order with documented variations. None of these is imported as a Korean rule. This project has no king piece, king-specific checks, fifteen-piece setup, Japanese color order, or Japanese camp restriction. The page also acknowledges products with different counts or no kings; that does not establish this project's Korean coordinate geometry.

## Reference licenses

The three local reference projects were inspected read-only. DIFF EQ, SOKOBAN and NUM GAME original code is MIT licensed. Their separate third-party notices remain authoritative for gint fonts, adapted infrastructure, fxSDK and linked runtime components. A copied substantial helper needs its corresponding MIT notice; a font atlas needs gint attribution/permission. SOKOBAN's third-party map rights are not covered by its MIT license, and no maps are needed here. Rules prose above is independently summarized; no source article, screenshot, diagram or product artwork is bundled.
