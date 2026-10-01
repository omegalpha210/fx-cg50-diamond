# Frozen synthetic fixtures

`save-v1-easy.bin` and `save-v1-hard.bin` are synthetic test archives produced
by DIAMOND's pre-NORMAL v1 codec at local baseline `1f26635`. They contain
generated opening boards, not personal saved games. Their persisted level bytes
are respectively 0 and 1. The compatibility test decodes and re-encodes them
byte for byte, then resumes directly into the same difficulty.

The legacy engine golden and source hashes preserve topology, legal moves,
paths, EASY choices/RNG, undo, v1 serialization, and the unchanged rule text.
The freeze check permits the explicit new difficulty validation/selector and
new AI profiles; it does not require the previous HARD search to remain fixed.
