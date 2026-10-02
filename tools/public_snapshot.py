#!/usr/bin/env python3
"""Audit/copy the explicit public source allowlist without local development history."""
import argparse
from pathlib import Path
import re
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
ROOT_FILES = {'CMakeLists.txt', '.gitignore', 'AGENTS.md', 'LICENSE',
              'THIRD_PARTY_NOTICES.md', 'README.md', 'README_KO.md'}
DOC_FILES = {'BETA4_AUDIT.md', 'GAME_RULES.md', 'BOARD_GEOMETRY.md', 'RULE_SOURCES.md',
             'STORAGE_FORMAT.md', 'DEVELOPMENT.md', 'AI_DESIGN.md',
             'AI_DIFFICULTY_AUDIT.md', 'POWER_AUDIT.md', 'UI_CONVENTIONS.md',
             'UI_BETA_AUDIT.md', 'HARDWARE_RETEST.md', 'BETA_VALIDATION.md',
             'PUBLICATION.md', 'RELEASE_NOTES.md', 'MEMORY.md',
             'LICENSE_AUDIT.md', 'ACCEPTANCE.md', 'UI_POLISH_BETA2.md', 'AI_MOVE_VISUALIZATION.md', 'UI_COLOR_AUDIT.md', 'USB_LIFECYCLE_AUDIT.md', 'ICON_TRAIL_POLISH.md'}
TEXT_SUFFIXES = {'.c', '.h', '.S', '.py', '.sh', '.txt', '.md', '.json', '.csv', '.yml', '.cmake'}

def allowed(name):
    p = Path(name)
    if name in ROOT_FILES:
        return True
    if p.parts[0] in {'src', 'include', 'tests', 'tools', 'assets'}:
        return not any(part in {'__pycache__', '.DS_Store'} for part in p.parts) and p.suffix != '.pyc'
    if len(p.parts) == 2 and p.parts[0] == 'docs' and p.name in DOC_FILES:
        return True
    if len(p.parts) >= 3 and p.parts[:2] == ('docs', 'screenshots'):
        return p.suffix in {'.png', '.md', '.csv', '.json'}
    return len(p.parts) >= 3 and p.parts[:2] in {('docs', 'third_party'), ('docs', 'ai')}

def files(root):
    return sorted(p for p in root.rglob('*') if p.is_file()
                  and p.parts[len(root.parts)] not in {'.git', 'build', 'dist'}
                  and allowed(p.relative_to(root).as_posix()))

def audit(paths, root):
    private = '/' + 'Users' + '/'
    selected = {p.relative_to(root).as_posix() for p in paths}
    for path in paths:
        name = path.relative_to(root).as_posix()
        assert allowed(name), f'not allowlisted: {name}'
        assert not path.is_symlink(), f'symlink: {name}'
        assert path.name not in {'DGSTATEA.dat', 'DGSTATEB.dat'}, f'personal save: {name}'
        if path.suffix in TEXT_SUFFIXES or path.name in ROOT_FILES:
            content = path.read_text()
            assert private not in content, f'private absolute path: {name}'
            assert not re.search(r'gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}', content), f'credential: {name}'
            if path.suffix == '.md' and not name.startswith('docs/third_party/'):
                for link in re.findall(r'\]\(([^)]+)\)', content):
                    if '://' in link or link.startswith(('#', '/', 'mailto:')):
                        continue
                    target = path.parent / link.split('#')[0]
                    relative = target.resolve().relative_to(root.resolve()).as_posix()
                    assert relative in selected or target.is_dir(), f'broken public link: {name} -> {link}'
    return len(paths)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--check-git', action='store_true', help='audit every tracked file, including history-free candidate')
    parser.add_argument('--output', type=Path, help='create a fresh public source tree; never overwrite an existing destination')
    args = parser.parse_args()
    assert allowed('tests/fixtures/save-v1-hard.bin') and not allowed('DGSTATEA.dat')
    assert not allowed('docs/reference-ui/sokoban-game.png') and not allowed('docs/validation/sh-build.log')
    assert not allowed('build/target/diamond') and not allowed('dist/DIAMOND.g3a')
    selected = files(ROOT)
    if args.check_git:
        names = subprocess.check_output(['git', 'ls-files', '-z'], cwd=ROOT).decode().split('\0')
        selected = [ROOT / name for name in names if name]
    count = audit(selected, ROOT)
    if args.output:
        destination = args.output.resolve()
        assert not destination.exists(), 'public destination already exists'
        destination.mkdir(parents=True)
        for path in selected:
            target = destination / path.relative_to(ROOT)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, target)
        audit(files(destination), destination)
    print(f'Publication PASS: {count} allowlisted files; no private paths, user saves, toolchains, caches, credentials or private reference images')

if __name__ == '__main__':
    main()
