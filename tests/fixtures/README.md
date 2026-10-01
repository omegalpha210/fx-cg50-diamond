# Frozen synthetic fixtures

`save-v1-easy.bin` and `save-v1-hard.bin` are synthetic test archives produced
by DIAMOND's pre-NORMAL v1 codec at local baseline `1f26635`. They contain
generated opening boards, not personal saved games. Their persisted level bytes
are respectively 0 and 1. The compatibility test decodes and re-encodes them
byte for byte, then resumes directly into the same difficulty.

`engine-golden-legacy.txt` retains the historical pre-NORMAL record.
`engine-golden-beta1.txt` and `ai-choices-beta1.csv` were captured before beta.2
edits at public baseline `f7e111c9c2a12243275875473fbbc30ecb34d628` / local
`42e5a55`. The CSV includes all 192 fixed-position choices (32 positions ×
2 modes × 3 levels), chosen endpoints/type/hops, RNG, legal counts, nodes,
completed depth and beam. Only host timing is excluded.

The beta.2 freeze check requires exact output equality, including HARD, and
byte-identical engine/game/AI/storage/power interfaces and sources. It also
freezes both engine/AI unit sources, including all 720 EASY golden choices,
and preserves the 21 rule strings. UI/controller files may implement the
requested setup/EXIT/visual behavior; persisted difficulty IDs and v1 codec
remain frozen.
