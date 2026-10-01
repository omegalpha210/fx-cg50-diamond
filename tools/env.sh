#!/usr/bin/env bash
DIAMOND_SDK_ROOT="${DIAMOND_SDK_ROOT:-$HOME/.local/diffeq-sdk}"
export DIAMOND_SDK_ROOT
export PATH="$DIAMOND_SDK_ROOT/prefix/bin:$DIAMOND_SDK_ROOT/prefix/share/fxsdk/sysroot/bin:$DIAMOND_SDK_ROOT/venv/bin:$PATH"
