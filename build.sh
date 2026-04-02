#!/usr/bin/env bash
set -e

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel "$(nproc)"
exec ./build/Pong
