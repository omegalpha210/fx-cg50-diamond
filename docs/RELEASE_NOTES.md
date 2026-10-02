# DIAMOND v0.1.0-beta.4

This experimental beta corrects Korean camp-entry rules, preserves old saves,
focuses HARD endgame play and shrinks the Main Menu icon uniformly.

- NEW uses V2: own HOME/GOAL membership permits each landing, including shared
  boundary holes; otherwise active opponents' HOME/GOAL forbids entry. All four
  boundary-row holes count. Inactive YELLOW camps remain open in 2P. The physical
  manual supports the restriction; shared-corner precedence is a documented
  project interpretation. The actual 73-node topology is unchanged.
- V1 saves resume with their original rules, board/order/difficulty/RNG/undo.
  RESTART keeps that revision; NEW uses V2. A/B transactions and record lengths
  are unchanged. The new transient AI history is never serialized.
- Every difficulty takes an immediate legal win before search or RNG. From
  seven goals, NORMAL/HARD use unique goal assignment and packing; HARD adds
  two plies within a 24,000-node limit. Bounded history reduces aimless reversal
  while keeping legitimate rearrangements legal.
- In 60 fixed positions HARD goal exits fall from 2 to 0; all-level immediate
  win misses are zero. Five old 1,200-ply stalls now finish with both V1 and V2.
  All 588 V2 matched/control games finish with zero illegal moves or crashes.
  2P NORMAL/EASY is 74:26, HARD/NORMAL 94:6, HARD/EASY 100:0; 3P mixed wins are
  EASY 22, NORMAL 50, HARD 72. These are harness results, not human ratings.
- Normal/selected icons use identical 92% affine scaling, one clear top row
  and 17 clear bottom rows. Bright yellow/light-green 2px/3px trails and
  117.1875ms-per-hop replay remain unchanged.

Strict host **16/16**, full UBSan **16/16**, strict SH **zero warnings**, 16 G3A
package checks, 145 actual renderer frames, camp/reachability and endgame audits.
AI workspace 9,895 bytes; controller 704 bytes;
largest own compiler frame 1732 bytes; one framebuffer.
ASan remains **NOT VERIFIED** because of the previously observed Darwin runtime
startup failure. Host timing is never calculator timing.

`DIAMOND.g3a`: **94,668 bytes**. SHA256:
`2db3242112391659e24901259f1b9c6e8136ec97867dd49ecb449949bb81908a`.

**HARDWARE TEST REQUIRED.** Icon/OS-label overlap, LCD contrast, native AI latency,
stack/heap high water, storage/power behavior and USB detection/dialog behavior
remain pending. The prior experimental USB handoff is included unchanged; no USB
fix is claimed. The owner explicitly authorized this experimental binary release
with those hardware checks pending.

See [full evidence](BETA4_AUDIT.md), [hardware checklist](HARDWARE_RETEST.md) and
[publication procedure](PUBLICATION.md). The exact public source is rebuilt before
upload, and release G3A/checksums are re-downloaded and compared after upload.
