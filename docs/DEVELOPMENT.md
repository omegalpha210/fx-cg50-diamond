# Development

The engine is the single source of move legality. Never add a second UI or AI
validator, Japanese king/camp rules, per-hole switch cases or a gameplay turn
limit to fix AI cycles. Original source and assets must remain public-safe.
The three older projects are read-only references.

## Build and inspect

```sh
bash tools/test.sh
bash tools/build.sh
cmake -S . -B build/ubsan -DDG_HOST=ON -DDG_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ubsan -j8
ctest --test-dir build/ubsan --output-on-failure
build/host/capture docs/screenshots
python3 tools/ui_captures.py
build/host/ai_audit --bench
build/host/ai_audit --stress 5000 128
build/host/ai_audit --selfplay 100 1200 0
bash tools/validate_beta.sh
```

The stress argument is per mode, hence 5,000 yields 10,000 positions and
20,000 selections. Selfplay runs three policies in each mode, hence 100 yields
600 games. The diagnostic 1,200-ply cap is not a game rule. Audit timing is
host CPU time. Hardware performance remains unmeasured.

`DG_HOST` builds portable core/UI and host tools; otherwise CMake uses the
installed fxSDK toolchain. Host warnings include `-Wconversion/-Wshadow` and
assertions remain enabled in Release. Target warnings are errors; SH compiler
stack reports use `-fstack-usage` and a 2,048-byte frame warning. No toolchain
directory is written by these scripts. The release identity is `@DIAMOND`;
storage uses only `DGSTATEA.dat` and `DGSTATEB.dat`.
The G3A release date defaults to `2026.1001.0000` via `DG_PACKAGE_DATE`, so
packaging does not acquire a different checksum just from the wall clock.

## Responsibility boundaries

- `src/core`: generated coordinates, navigation/distances and move BFS.
- `src/game`: turn order, committed state, victory, restart and human undo.
- `src/ai`: one bounded static workspace, seeded EASY and shared NORMAL/HARD searches.
- `src/ui`: platform-independent controller and rectangle-based renderer.
- `src/storage`: explicit codec, A/B transaction and POSIX/BFile adapters.
- `src/platform`: real gint keys, cooperative cancellation, MENU/OFF and idle.
- `tools`: deterministic generators, actual-render captures and host audit.

`DgMove` stores endpoints, type and shortest hop count. `dg_find_move()` obtains
its representative `DgPath` from the same BFS for preview/animation. Search
branches deduplicate endpoints. Runtime CPU animation is uncommitted until the
last frame; cancellation discards the pending move and pending RNG.

The app uses one native framebuffer. Only actual DOWN/HOLD input resets idle;
draws, timer wakeups, warnings and search callbacks do not. A single scheduler
timer wakes foreground idle/animation work, with RTC fallback. AI never runs
in the timer callback. The OS supplies dim/APO durations, read through the
small scalar syscalls retained with attribution. MENU/OFF use gint lifecycle
APIs in this order: checkpoint committed state → close storage handles →
pause scheduling and restore brightness → enter the OS. Save/close failures
retain RAM and block the system handoff for retry.

See [BETA_VALIDATION.md](BETA_VALIDATION.md) and [MEMORY.md](MEMORY.md) for beta
measurements. [AI_DIFFICULTY_AUDIT.md](AI_DIFFICULTY_AUDIT.md) gives reproducible
fixed-position/reference and matched-tournament commands. Run
`python3 tools/public_snapshot.py --check` before publication; never include
private reference captures or raw local logs. Stable v1 level IDs are EASY=0,
HARD=1, NORMAL=2; menu display order is separate.
