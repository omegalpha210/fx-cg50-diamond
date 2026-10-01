# Native control-flow shim

`tests/test_native.c` includes the real `src/platform/main.c` with its entry
point renamed. Tests call its actual static key translation, input barrier,
AI cancellation callback, idle processing, and system action functions.
The real game, AI, UI, renderer, power policy and archive encoder are linked.

The headers in `gint/` provide narrow host doubles for the SDK calls used by
that file. Mock event queues, held-key state, RTC ticks, brightness, timers,
OS entry points and storage hooks expose ordering and fault behavior. No
calculator emulator, native storage I/O, or hardware timing is represented.
Storage transaction correctness has its own tests.

Host CMake must put `tests/native` before other include paths and link the
native test executable against `diamond_ui`. SDK headers and this test are
excluded from the calculator target. The native infinite event loop itself
is reviewed, while its actual constituent functions are exercised directly.

Validated cases include SHIFT tap/release and held SHIFT plus AC/ON; held-key
barriers and DOWN/UP/HOLD handling; the bounded 32-event callback drain; CPU
MENU/OFF/EXIT cancellation without committing board/RNG/turn changes; save
and handle-cleanup failures preventing OS entry; ETMU/RTC pause and resume;
and dim/APO with no idle reset from rendering or timer callbacks.

Power coverage includes all 48 accepted/unsupported Backlight/APO combinations,
querying both scalars only in the OS world, refresh after MENU, and the actual
`char` Backlight Duration return contract. Brightness doubles match installed
gint's `uint16_t r61524_get()` / `r61524_set(int, uint16_t)` signatures. Tests
exercise one brightness restore and one gameplay action for a wake key; no
idle reset from redraw, warning changes, animation, save or AI work; an ISR
that only sets the wake flag; and foreground APO checkpoint/cleanup failures
that retain RAM and cannot produce a per-tick retry loop. Cooperative system
key cancellation runs for EASY, NORMAL and HARD in both player counts.

All calculator interaction remains **HARDWARE TEST REQUIRED**.
