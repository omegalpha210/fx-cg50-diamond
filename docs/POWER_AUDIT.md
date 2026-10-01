# Power, dim and APO audit

Baseline: `1f26635b264076d0af894486a24598d11cb109fb`. All four power paths already existed at that baseline. This audit checked their actual native implementation, read the reference source and installed SDK, and expanded regression coverage. No dim/APO state-machine rewrite was needed. The Backlight Duration query declaration now matches its documented `char` return type; the native test doubles now match gint's 16-bit LCD register signatures. This is ABI alignment and stronger evidence, not a claim that a calculator power defect was reproduced.

| Function | Baseline implementation | Current flow |
| --- | --- | --- |
| Manual SHIFT+AC/ON | Present | Physical SHIFT/AC sequence → foreground app OFF → checkpoint → supported poweroff |
| MENU | Present | Foreground app MENU → checkpoint → storage cleanup → supported CASIO Main Menu |
| Inactivity dim | Present | Independent RTC idle accumulation → actual LCD PWM dim → exact PWM restoration on physical input |
| Automatic power off | Present | Idle deadline → cooperative search cancellation when needed → foreground checkpoint → supported poweroff |

## OS settings and fallback

`src/platform/main.c` runs `read_power()` through `gint_world_switch(GINT_CALL(...))` at entry and after MENU/poweroff returns. `src/platform/power_syscalls.S` contains two small read-only scalar queries. The dispatcher address is the documented syscall trampoline, rather than a guessed setting-data location. The linked stubs were disassembled: syscall number in r0, `0x80020070` in r2, tail jump with a NOP delay slot.

| Setting | Query contract | Accepted OS values | Explicit fallback |
| --- | --- | --- | --- |
| Backlight Duration | `char GetBacklightDuration()`, index `0x12D9`, 30-second units | `1`, `2`, `6` → 30, 60, 180 seconds | 60 seconds |
| Auto Power Off | `int GetAutoPowerOffTime()`, index `0x1E91`, minutes | `10`, `60` → 10, 60 minutes | 10 minutes |

Each scalar is validated independently. A failed/unsupported value falls back for that field while a valid other field remains OS-derived. The fallback is a project policy reused from NUM GAME; it is not reported as the user's OS setting. There is no duplicate saved add-in power preference or OS setting write.

Primary ABI evidence: pinned libfxcg [system declarations](https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/include/fxcg/system.h), [Backlight Duration stub](https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/libfxcg/syscalls/GetBacklightDuration.S), [APO stub](https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/libfxcg/syscalls/GetAutoPowerOffTime.S), and [dispatcher macro](https://github.com/Jonimoose/libfxcg/blob/daa76d9e37758ab5b9b0f614b7e0aa6acfbe2e18/include/asm.h). No libfxcg implementation or dependency was added to the add-in.

## Installed API and actual dim

Installed gint is 2.11, revision `badbd0fd2bd8ac796fd55d49b93691741bd8a139`. Its actual headers and implementation were read: `include/gint/gint.h`, `include/gint/rtc.h`, `include/gint/timer.h`, `include/gint/keyboard.h`, `include/gint/drivers/keydev.h`, `include/gint/drivers/r61524.h`, and `src/r61524/r61524.c`.

- `uint16_t r61524_get(int ID)` and `void r61524_set(int ID, uint16_t value)` are the installed public register APIs. The driver's own brightness implementation uses PWM register `0x5A1`; its brightness table includes `0x14` as the low value.
- DIAMOND saves the exact original `0x5A1` value once per dim interval and writes `min(original, 0x14)`. It restores that saved value once on a wake input or before successful OS entry. A very low original PWM value is preserved. No black rectangle is used to imitate dimming.
- This is PWM dimming with preserved LCD/port configuration. Visual equivalence to CASIO's system dim level is an implementation inference from the installed driver and NUM GAME pattern; exact brightness and battery savings remain **HARDWARE TEST REQUIRED**.
- `gint_osmenu()` and `gint_poweroff(true)` are the installed supported world-transition helpers. The latter uses the poweroff/logo behavior appropriate to AC/ON. Temporary brightness is restored before either helper is invoked.

## Activity and foreground ordering

`rtc_ticks()` counts 128-Hz ticks since midnight. `DgPower` handles midnight wrap and accumulates idle time independently. Only observed physical `KEYEV_DOWN` or `KEYEV_HOLD` events reset that accumulation. Releases, empty wake events, redraws, warnings, animations, save completion and search work do not reset it. A modifier or unused physical key also counts even when it produces no gameplay action. A physical key arriving at the deadline takes priority over automatic expiry.

A dimmed wake key restores brightness and passes through normal key translation once. Non-direction HOLD events cannot duplicate an EXE/menu/modal action. Direction HOLD follows the existing repeat policy. Input barriers retain the established held-key protections.

The allocated 20 ms gint timer is the normal wake source; `rtc_periodic_enable(RTC_64Hz, ...)` is the allocation-failure fallback. Their `pulse()` callback only sets a wake flag. It performs no RTC query, flash write, power call, rendering, game mutation or AI work. Native foreground functions perform all power decisions. This is the same shared
wake source, with finer visual polling; RTC-based dim/APO values, physical-input
activity rules and OS handoff order are unchanged.

During CPU search, the bounded cooperative callback reads physical events, advances idle time and sets a pending action. It returns cancellation before the app dispatches OFF/MENU. Only the last committed board/turn/RNG is checkpointed; partial search results are discarded. Beta.3 replay is visual only: its final move
has already committed, so skipping it retains final board/turn/RNG. Winning
commits save the unchanged inactive v1 tombstone before replay. Successful OS entry follows this order:

1. Cancel pending search or finish visual replay; checkpoint dirty final committed state.
2. Clean up native storage handles.
3. Pause the app wake source and restore any saved brightness.
4. Drain/block held input, then invoke the supported MENU or OFF helper.
5. If execution returns, query current OS settings again, apply the input barrier and resume the wake source.

Checkpoint failure retains dirty RAM and a visible retry notice, with no OS call. Handle-cleanup failure also prevents OS entry and keeps the foreground clock active. The idle state resets after emitting an APO request, so following wake ticks do not retry flash writes in a hot loop; the next automatic attempt waits another full APO interval. Manual retry remains available.

## Read-only reference evidence

| Reference | Inspected revision and source | Relevant behavior |
| --- | --- | --- |
| DIFF EQ | `a6c9659342e2ab7661072cf5916a4ba6ee81e96a`; `src/power.c`, `docs/audits/POWER_SAFETY_AUDIT.md` | Same scalar syscall IDs, `char` duration ABI, supported MENU/OFF helpers, installed-driver low PWM evidence. Its unsupported-setting policy disables that action; DIAMOND retains the requested NUM GAME fallback. |
| NUM GAME | `825b5561914a198629a0490fa229c956c3abdfc6`; `src/main.c`, `src/core/runtime.c`, `src/power_syscalls.S`, `docs/POWER_TIMER_AUDIT.md`, `tests/test_native.c` | Exact 60-second/10-minute fallback, DOWN/HOLD-only activity, actual PWM save/dim/restore, foreground checkpoint and timer/RTC wake strategy. |
| SOKOBAN | `2492545230d49ccc3284bc48edb3ee223111f88f` | Read-only status checked as required; power source decisions came from DIFF EQ and NUM GAME. |

Reference status was captured before and after inspection. NUM GAME and SOKOBAN remained clean. DIFF EQ retained the same pre-existing untracked development file; no tracked reference file or status entry was changed.

## Host verification and hardware limits

`tests/test_power.c` covers 48 combinations of valid and failed/unsupported scalar values, exact dim/APO boundaries, input priority, one restoration, midnight/fractional RTC accumulation and finite repeat policy. Existing malformed-state checks remain enabled.

`tests/test_native.c` compiles the actual native static functions with controlled SDK/storage doubles. Its beta.1 49,425 checks include all 48 OS query combinations in the OS world, refresh after MENU, dim/restore values and call counts, one wake gameplay action, internal redraw/warning/save/animation/search activity, flag-only ISR behavior, all three difficulty profiles' system-key cancellation in 2P/3P, successful checkpoint-before-off ordering, and finite save/cleanup-failure recovery. The `power` and `native` CTest cases pass in both strict Release and UBSan builds with assertions enabled. The target build compiles with strict warnings and its package checks pass; the final release report records the exact rebuilt artifact.

Beta.2 preserves these cases and adds clock-stepped visual checks; the current
count is in [BETA_VALIDATION.md](BETA_VALIDATION.md). THINKING redraws use the
existing foreground hook and do not count as input or change power state semantics.

**HARDWARE TEST REQUIRED:** actual OS query values across installed OS versions; 30/60/180-second LCD dim; exact brightness restoration after physical input; 10/60-minute APO and power-on return; MENU and SHIFT+AC/ON; search cancellation before APO; timer/RTC resource availability; long play with animation and highlights without input. Host timing is not calculator timing.

If both timer allocation and RTC periodic allocation fail, the existing native code displays `IDLE TIMER UNAVAILABLE`. Autonomous idle deadlines cannot be promised in that unsupported wake-source condition; manual MENU/OFF remains available. The 24-hour RTC counter cannot infer multiple whole days between unobserved samples. Hardware suspend/wake, storage failure behavior and LCD/PWM electrical behavior are not emulated by host tests.
