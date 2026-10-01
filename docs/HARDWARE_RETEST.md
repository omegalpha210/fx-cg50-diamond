# fx-CG50 beta hardware retest

All 28 items are **PENDING — HARDWARE TEST REQUIRED**. Host tests, renderer
pixels and mocked native APIs do not verify actual calculator operation.
Record OS version, system dim/APO values, package checksum and observations.

| # | Check | Acceptance |
|---:|---|---|
| 1 | PLAYER (YOU ARE RED) | One baseline, red annotation, no overlap |
| 2 | AI icon readability | Black AI centered in GREEN/YELLOW circles |
| 3 | 2P FIRST HUMAN/AI | Wording, selection and actual first turn agree |
| 4 | LEVEL three faces | Green smile / yellow neutral / red angry; clamp |
| 5 | EASY | Legal moves; expected seeded easy play |
| 6 | NORMAL | Legal shallow-search play |
| 7 | HARD | Legal deeper-search play |
| 8 | Piece fill/readability | Thin outline and full-color interior in both scales |
| 9 | Yellow/green/red distinction | Legible actual LCD colors |
| 10 | Cyan Assist destination | Filled empty hole, visible cursor, correct legal set |
| 11 | Assist OFF | No cyan; invalid move remains a red warning |
| 12 | RULES black/white | All RULES keys black with white text |
| 13 | Restart | Confirmation and same difficulty/seed/order |
| 14 | Undo | One human turn plus replies; no second undo |
| 15 | Zoom | Cursor/selection/navigation and no old cross markers |
| 16 | NORMAL AI latency | Measure 2P/3P opening, middle, congestion; cancellation |
| 17 | HARD AI latency | Measure same positions; target about 1–2 s, unverified |
| 18 | 3P turn order | New randomizes only AI slots; restart/resume preserve |
| 19 | Resume old EASY | Native v1 value 0 retains EASY |
| 20 | Resume old HARD | Native v1 value 1 retains HARD |
| 21 | New NORMAL resume | Cold load value 2, undo and A/B recovery |
| 22 | System dim | Actual Backlight Duration query and configured timeout |
| 23 | Key brightness restore | Original brightness restored once, key delivered once |
| 24 | APO | Actual configured idle timeout, search cancel, save before OFF |
| 25 | MENU | Save, real OS menu, return and input barrier |
| 26 | SHIFT+AC/ON | Save, supported OFF, wake and input barrier |
| 27 | Long play | Persistence, power, cancellation, runtime stack/heap stability |
| 28 | Result/new | Correct winner; NEW keeps NORMAL when selected |

Do not mark an item passed without physical evidence. A latency target is not a
measured promise. Forced power interruption, storage faults and brightness API
behavior must also be observed during the relevant checks.
