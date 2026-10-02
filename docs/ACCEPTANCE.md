# Acceptance evidence — beta.4

[Complete audit](BETA4_AUDIT.md), [validation](BETA_VALIDATION.md),
[storage specification](STORAGE_FORMAT.md) and [hardware checklist](HARDWARE_RETEST.md)
record the current milestone. Calculator-only claims remain HARDWARE TEST REQUIRED.

V2 uses player-relative own HOME/GOAL precedence on all six shared corners,
then excludes active opponents' camps at every landing. All HOME/GOAL memberships,
four-hole boundaries, 50 start-to-all-goal reachability traversals, intermediate
jump exclusions, legal goal exit, Assist and nonmodal warnings are checked.
The original 73-node topology and neighbor/navigation tables are byte-identical.
The independent coordinate oracle compares complete move endpoint sets on 5,000
reachable V1/V2 boards. Retracing remains allowed; final no-op turns remain excluded.

V1 active archives preserve board/order/difficulty/RNG/undo and exact active
re-encoding. NEW uses V2; RESTART retains its game's revision. Both formats use
one movement engine and the original A/B transaction. Version, revision, CRC,
length, fault injection, interrupted-write recovery and mixed-version recovery
are covered. Trail and 12-turn AI histories remain transient and never serialized.

All-level immediate wins, GREEN's newly vacated last goal, 60 matched synthetic
endgames, an independent assignment oracle, color rotation, 20 progress/reversal
fixtures and 578 legal rearrangements are checked. HARD's 60-case V2 audit has
zero goal exits and one assignment regression, versus beta.3's two/two.
All five historical 1,200-ply stalls finish with the new AI under V1 and V2.
All 588 V2 matched/control games finish, with zero illegal moves or crashes.
Reported improvements are harness evidence, not human ratings or optimal play.

CPU moves commit once before the unchanged representative-path replay. The
73-byte pre-board, transient paths, system-request skip/checkpoint behavior,
post-replay RESULT, 15-tick hop cadence and 40-tick THINKING cadence remain tested.
No visual work consumes RNG, changes search or resets idle. Trail geometry,
colors, native USB/power code and the save transaction are protected by a
13-file byte-preservation manifest. The old whole-engine UI-only freeze is
explicitly superseded by this authorized rules/AI milestone; historical files
are retained instead of rewriting their measured results. The 720 V1 EASY
opening/midgame choice/RNG/evaluation goldens still pass.

PLAYER/SETUP entry behavior, clamps, F1 RESUME, F1 restart, F2 one-decision undo,
F5 zoom and normal versus busy EXIT behavior remain covered. Goal/cursor/hole
and HUD geometry, bright yellow/light-green trails, 2px/3px lines, shared lanes,
arrow clearance and cyan Assist retain their actual-pixel tests.

145 common-renderer captures include seven new rules/Assist/endgame views and
the existing 26-frame five-hop historical V1 route. Six new camp overlays show
exact IDs and shared boundaries. Normal/selected icon bounds match after one
uniform 92% affine scale; native/8× and label-mock comparisons are generated.
Actual OS-label overlap remains unverified on hardware.

Strict host and UBSan, clean SH zero-warning build, package checks, memory audit
and a rebuild of the exact public source precede upload. Release G3A/checksums
must be downloaded again and compared byte for byte. ASan is NOT VERIFIED.
USB remains experimental, with calculator connection detection/dialog behavior
pending; the owner explicitly authorized beta.4 binary publication in that state.
