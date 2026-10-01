# DIAMOND state format, version 1

The independent files `DGSTATEA.dat` and `DGSTATEB.dat` contain generations of
one logical archive, comprising global Assist and at most one unfinished game.
These names were checked against the three read-only reference projects.
An inactive archive is a checksummed tombstone that retains Assist and suppresses
an older unfinished game. Completed boards are never resumable archives.

All integers are explicitly little-endian. No C structure, padding, pointer,
cursor, zoom, animation, search workspace or status text is serialized.
`DG_SAVE_BYTES` is a 256-byte decode limit; actual records use 24, 124 or 208 bytes.
All lengths are even for safe BFile writes.

| Offset | Bytes | Field |
| --- | ---: | --- |
| 0 | 8 | ASCII `DGSAVE01`, unique to DIAMOND |
| 8 | 2 | Version, unsigned 16-bit, exactly 1 |
| 10 | 2 | Exact total record length |
| 12 | 4 | Unsigned generation |
| 16 | 4 | CRC-32/ISO-HDLC, complete record with bytes 16–19 treated as zero |
| 20 | 1 | Assist: 0 or 1 |
| 21 | 1 | Active unfinished game: 0 or 1 |
| 22 | 1 | Undo present: 0 or 1, must be 0 when inactive |
| 23 | 1 | Reserved, zero |
| 24 | 1 | Player count, 2 or 3 (active records only) |
| 25 | 1 | Stable difficulty ID: Easy 0, Hard 1, Normal 2 |
| 26 | 1 | Human turn slot, zero based |
| 27 | 3 | Color turn order; unused 2-player slot is 255 (DG_NONE) |
| 30 | 2 | Reserved, zero |
| 32 | 4 | Game seed |
| 36 | 4 | Initial gameplay RNG; restart restores this exact value |
| 40 | 84 | Current committed position |
| 124 | 84 | Optional one-human-decision undo position |

A position stores 73 occupancy bytes (`EMPTY=0`, `RED=1`, `YELLOW=2`, `GREEN=3`),
then turn slot, winner, a zero reserved byte, unsigned RNG and unsigned turn count.
An inactive record ends at byte 24. An active record without undo ends at byte
124; an active record with undo ends at byte 208. An undo flag and any other
length combination are rejected. Absent undo state is zero initialized in RAM.

Engine validation checks player count, difficulty, human slot, exact active-color
permutation, ten pieces of each participant, no pieces of absent participants,
turn index, nonzero RNG/initial RNG, winner consistency and undo validity. The
archive additionally requires an unfinished current position. All reserved
fields, flags, version, length, magic and checksum are checked before acceptance.

NORMAL adds the unused ID 2 without changing the v1 layout or version. Existing
EASY/HARD archives retain values 0/1 and resume directly at their original
difficulty; no migration rewrites them. The display order EASY/NORMAL/HARD is
separate from serialized IDs. Synthetic pre-NORMAL archives in `tests/fixtures`
must decode and re-encode byte for byte. Older add-in builds do not support
NORMAL archives and will reject ID 2 rather than misinterpret it.

## A/B transaction

1. Validate the proposed archive, then read and validate both existing files.
2. Abort on any read I/O error, because that copy's freshness is unknown.
3. Preserve the newest valid copy; replace only the other copy. With no valid
   copies, select A. Increment the newest disk generation, or start at 1.
4. Close the fully written file, reopen it, compare every byte and decode again.
5. Update the caller's generation only after this verification succeeds.

The native writer may delete/recreate the selected older file, but never removes
the newest valid file. Interrupted writes and invalid checksums are ignored on
load, leaving the prior verified copy recoverable. A save that fails after all
bytes reached storage can have an ambiguous outcome; the untouched previous
copy remains available, and next load chooses the newest valid complete record.
Generation order uses unsigned serial arithmetic across wraparound; equal or
exactly half-range-apart generations deterministically prefer A.

Load prefers the newest valid archive, including tombstones. If the companion
file is present but invalid or unreadable it reports `DG_LOAD_RECOVERED`.
A missing companion is normal, especially after the first successful save.
Both missing returns `DG_LOAD_ABSENT`; no valid copy plus an I/O error returns
`DG_LOAD_IO_ERROR`; otherwise no valid copy returns `DG_LOAD_INVALID`. These
unsuccessful loads return fresh defaults: Assist on and no active resume.

The injected `DgStorageIO.read` returns `-2` for absent, `-1` for I/O failure or
the exact nonnegative file size. Oversized files return their size without
writing beyond the supplied buffer. `write` replaces only the requested slot
and succeeds only after all bytes are written and the handle is closed.
Callbacks are synchronous, main-thread only. The target wraps the complete
transaction in `gint_world_switch()` and uses BFile only in the OS world.
`dg_storage_cleanup()` retries a retained failed-close handle in the OS world;
native MENU/OFF must remain blocked until this cleanup succeeds.
Host files are created in the process's working directory, with file and
directory `fsync`; host failure injection uses isolated memory-backed files.

Checkpoint only committed state on gameplay exit, MENU/OFF, new/restarted game,
and completion/active-resume clear. Cursor redraw and CPU search never write.
Hardware persistence, power interruption, cold resume and OS lifecycle behavior
are **HARDWARE TEST REQUIRED**; host tests do not establish calculator behavior.
