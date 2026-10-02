# Public beta publication

Repository: [omegalpha210/fx-cg50-diamond](https://github.com/omegalpha210/fx-cg50-diamond).
This milestone publishes [v0.1.0-beta.4](https://github.com/omegalpha210/fx-cg50-diamond/releases/tag/v0.1.0-beta.4)
as an experimental prerelease. It is a normal descendant of public beta.3
`54e63bce22ba41b6a3cc4ef518e5b323c43e5768`; previous releases/tags are preserved.
Private local development history is excluded. No force push is used.

The explicit allowlist includes original source/tests/tools/assets, EN/KO
README, licenses, beta.4 audit, own captures and compact AI evidence. It excludes
private paths, personal saves, toolchains, raw build logs/maps/ELFs, caches and
reference-project images. `tools/public_snapshot.py --check` audits the snapshot;
`--check-git` audits every tracked public file. The full existing public ancestry
is scanned for forbidden paths, credentials, personal saves and symlinks before
copying the reviewed snapshot into a clean public checkout.

The exact candidate source runs strict host, complete UBSan, V2 smoke matches,
endgame/rule audits, strict SH, package checks and actual renderer captures.
Package and capture bytes are compared to local validation. The published tag
identifies that rebuilt source; obtain its identity with
`git rev-parse v0.1.0-beta.4^{commit}`. Release assets contain only DIAMOND.g3a and
SHA256SUMS.txt. Both are re-downloaded, compared byte for byte and checked against
GitHub asset digests. Final release notes record those verification results.

The owner explicitly authorized this beta.4 binary release with physical
verification pending. It includes the prior experimental conservative USB
handoff unchanged; this is not a claim that USB detection or connection-dialog
behavior is fixed. Icon label clearance, LCD readability, AI latency and
power/storage/USB behavior remain HARDWARE TEST REQUIRED. Host timing never
stands in for calculator timing.
