# Public beta snapshot

The authenticated publishing account is `omegalpha210`. The repository name is
`fx-cg50-diamond`; the first prerelease tag is `v0.1.0-beta.1`.

Before publication, all five existing local commits were inspected. Two contain
personal absolute paths and private reference screenshots. The local history is
preserved. A separate allowlisted public snapshot excludes those commits and
private files; it does not rewrite the local development branch.
Future releases should update the existing public ancestry from an audited
snapshot rather than force-push the private local history.

`tools/public_snapshot.py --check` scans the public allowlist for private paths,
credentials, symlinks, user saves and prohibited files. `--output` copies a fresh
source tree; `--check-git` checks every tracked public file. The public tree
contains original source/tests/tools, retained licenses, EN/KO README, original
icons, actual DIAMOND renderer captures and curated audit reports. It excludes
legacy raw build logs, temporary AI logs, private reference images, reference
maps, toolchains, caches and personal save files.

The public candidate is rebuilt with strict host tests, UBSan, AI/save/power
checks and a clean SH build. Renderer captures are regenerated from the same
candidate. The G3A and checksum file uploaded to the prerelease come from that
source. Release assets are downloaded again and compared byte for byte and by
SHA256. Final release identity and verification are recorded after upload.
