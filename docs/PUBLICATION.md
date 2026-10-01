# Public beta publication

Repository/account: `omegalpha210/fx-cg50-diamond`. Current prerelease:
[v0.1.0-beta.3](https://github.com/omegalpha210/fx-cg50-diamond/releases/tag/v0.1.0-beta.3).
Its public source is a normal descendant of beta.2 `775310a178da07b275af3ecf0817339cbbae19d2`,
which descends from beta.1 `f7e111c9c2a12243275875473fbbc30ecb34d628`.
Old commits/tags/releases remain unchanged. Private local development history
stays local; no force push or unrelated new root is used.

The initial beta.1 snapshot excluded personal paths and private reference images.
Beta.3 copies the reviewed allowlist into a clean checkout of the existing public
branch. `tools/public_snapshot.py --check` scans source/docs for private paths,
credentials, symlinks, personal saves and broken relative links. `--check-git`
audits every tracked public file. The full public ancestry is also audited.
New AI_MOVE_VISUALIZATION/UI_COLOR_AUDIT reports are explicitly allowlisted;
original code/tests/tools/assets, EN/KO README, own captures and retained licenses
are included. Toolchains, raw logs/maps/ELFs/caches, personal saves and private
reference screenshots remain excluded. Reference projects remain read-only.

The clean exact source runs strict host, full UBSan, all choice/path goldens,
representative selfplay, UI/native/storage/power tests, strict SH, package checks
and actual renderer captures. Local and candidate artifact/capture bytes match.
The candidate-built G3A and SHA256SUMS are uploaded, re-downloaded and compared
byte-for-byte, with checksum verification and both GitHub SHA256 digest fields.
Release notes record post-upload verification. Exact public identity is obtained
with `git rev-parse v0.1.0-beta.3^{commit}`; the delivery report gives the commit.
All native behavior remains HARDWARE TEST REQUIRED.
