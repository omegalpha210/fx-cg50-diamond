#!/usr/bin/env bash
# Run from a fresh public snapshot to verify the release source.
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/env.sh
mkdir -p build
python3 tools/public_snapshot.py --check
python3 tools/generate_board.py --check
python3 tools/font_data.py --check
cmake -S . -B build/host -DDG_HOST=ON -DCMAKE_BUILD_TYPE=Release > build/host-configure.log 2>&1
cmake --build build/host -j8 > build/host-build.log 2>&1
ctest --test-dir build/host --output-on-failure | tee build/host-tests.log
cmake -S . -B build/ubsan -DDG_HOST=ON -DDG_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug > build/ubsan-configure.log 2>&1
cmake --build build/ubsan -j8 > build/ubsan-build.log 2>&1
ctest --test-dir build/ubsan --output-on-failure | tee build/ubsan-tests.log
bash tools/build.sh > build/sh-build.log 2>&1
if rg -n 'warning:' build/host-build.log build/ubsan-build.log build/sh-build.log; then
  exit 1
fi
python3 tools/verify_g3a.py dist/DIAMOND.g3a
python3 tools/ui_captures.py
python3 tools/memory_report.py > build/memory-report.log
python3 tools/public_snapshot.py --check
