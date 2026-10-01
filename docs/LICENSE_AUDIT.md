# Public beta provenance and license audit

Audited the actual source/assets and retained local license texts on 2026-10-01.
The public snapshot keeps every notice in `docs/third_party`; package-linked
runtime permission is separate from DIAMOND's original MIT code.

| Material | Provenance / distribution decision |
|---|---|
| Board coordinates, legality, game, AI, UI | DIAMOND source; generated mathematical lattice; original MIT |
| Icons and game screenshots | Generated original geometry and actual shared renderer; MIT; no board photo |
| Rules | Independent short summaries with source links; no article, wiki text or illustration assets |
| DIFF EQ helper reference | Pinned `a6c9659342e2ab7661072cf5916a4ba6ee81e96a`; retained MIT notice |
| SOKOBAN font/text helpers | Pinned `2492545230d49ccc3284bc48edb3ee223111f88f`; retained MIT notice; no maps/images |
| NUM GAME syscall/lifecycle reference | Pinned `825b5561914a198629a0490fa229c956c3abdfc6`; retained MIT notice |
| gint 2.11 runtime/font | Pinned `badbd0fd2bd8ac796fd55d49b93691741bd8a139`; author's use/share/modify permission retained in gint README |
| Font atlas and generated glyphs | Exact gint atlas SHA256 `0e7f56f30e021e16053360b972adc0b020c7d8a2d545fbd883bc8e7d870413c8`; PNG is regeneration input; 1,140-byte table compiled |
| fxSDK packaging tools | External MIT tools; retained notice; tools are not bundled |
| FxLibc / Grisu | SDK-linked C runtime; retained CC0 and separate MIT notices |
| OpenLibm | SDK runtime dependency; retained combined upstream notices |
| GCC support runtime | Retained GPLv3 and GCC Runtime Library Exception 3.1 |

No proprietary OS font, CASIO firmware image, Korea Board Games article,
Wikipedia page, board illustration, Sokoban map, downloaded product image or
private reference capture is included. Reference projects were read-only.
Synthetic binary save fixtures are generated test data, not user saves.
Compiler/SDK binaries and build caches are excluded.

This records inspected provenance and included terms, rather than attributing
all dependencies to the project's MIT license. [Third-party notices](../THIRD_PARTY_NOTICES.md)
identify each retained license. The exact public-source build and package audit
are recorded in [BETA_VALIDATION.md](BETA_VALIDATION.md).
