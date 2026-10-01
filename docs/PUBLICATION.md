# Public beta publication

Repository/account: `omegalpha210/fx-cg50-diamond`. The current prerelease is
[v0.1.0-beta.2](https://github.com/omegalpha210/fx-cg50-diamond/releases/tag/v0.1.0-beta.2).
Its source is a normal descendant of beta.1 commit
`f7e111c9c2a12243275875473fbbc30ecb34d628`. The beta.1 commit, annotated tag
and release stay unchanged. Private local development history remains local.

The initial beta.1 snapshot excluded old commits with personal absolute paths
and private reference screenshots. Beta.2 copies the audited allowlist into a
checkout of the existing public branch and commits on that ancestry; no force
push, tag movement or separate unrelated root is used.

`tools/public_snapshot.py --check` scans allowed source/docs for paths,
credentials, symlinks, user saves and broken local links. `--output` makes a
fresh file snapshot; `--check-git` audits every tracked public file. Original
code/tests/tools/assets, EN/KO README, own captures and retained licenses are
included. Toolchains, caches, raw logs/maps/ELFs, private reference screenshots
and personal saves are excluded. New ACCEPTANCE/UI_POLISH_BETA2 reports are
explicitly allowlisted after public-content review.

The clean exact source candidate runs strict host, UBSan, AI fixed-choice and
selfplay regression, save/power/native tests and strict SH compilation. Captures
are regenerated and compared to the local output. G3A and SHA256SUMS come from
that tree. Both uploaded assets are downloaded again and compared byte-for-byte,
with checksum-file verification and GitHub SHA256 digests. Release notes record
verification after upload. Source identity is reproducible with
`git rev-parse v0.1.0-beta.2^{commit}`; the final delivery gives the exact commit.
