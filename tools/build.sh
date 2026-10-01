#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source tools/env.sh
python3 tools/generate_board.py --check
cmake -S . -B build/target -DCMAKE_MODULE_PATH="$DIAMOND_SDK_ROOT/prefix/lib/cmake/fxsdk" -DFXSDK_CMAKE_MODULE_PATH="$DIAMOND_SDK_ROOT/prefix/lib/cmake/fxsdk" -DCMAKE_TOOLCHAIN_FILE="$DIAMOND_SDK_ROOT/prefix/lib/cmake/fxsdk/FXCG50.cmake"
cmake --build build/target -j8
python3 tools/verify_g3a.py dist/DIAMOND.g3a
(cd dist && shasum -a 256 DIAMOND.g3a > SHA256SUMS.txt)
