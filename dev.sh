#!/usr/bin/env bash
set -e

BUILD_TYPE=
PONG_DEV=
RUN=0

for arg in "$@"; do
    case $arg in
        --release) BUILD_TYPE=Release; PONG_DEV=OFF ;;
        --debug)   BUILD_TYPE=Debug;   PONG_DEV=ON  ;;
        --run)     RUN=1 ;;
    esac
done

usage() {
    echo "Usage: ./dev.sh --debug|--release [--run] [--clean]"
    echo ""
    echo "  Build:"
    echo "    --debug              Debug build with PONG_DEV=ON  →  build/debug/"
    echo "    --release            Release build                 →  build/release/"
    echo ""
    echo "  Options:"
    echo "    --run                Run after build (skips build if binary exists)"
    echo "    --clean              Delete all of build/ and the symlink"
    echo "    --clean --debug      Delete only build/debug/"
    echo "    --clean --release    Delete only build/release/"
    echo ""
    echo "  Other:"
    echo "    -h                   Show this help"
}

if [[ " $* " == *" -h "* || "$1" == "-h" ]]; then
    usage; exit 0
fi

if [[ " $* " == *" --clean "* ]]; then
    if [[ " $* " == *" --release "* || " $* " == *" --debug "* ]]; then
        BUILD_DIR=build/$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')
        rm -rf "$BUILD_DIR"
        [[ -L compile_commands.json && "$(readlink compile_commands.json)" == "$BUILD_DIR/compile_commands.json" ]] && rm -f compile_commands.json
        echo "Cleaned $BUILD_DIR"
    else
        rm -rf build
        rm -f compile_commands.json
        echo "Cleaned all"
    fi
    exit 0
fi

if [[ -z "$BUILD_TYPE" ]]; then
    usage; exit 1
fi

BUILD_DIR=build/$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

if [[ $RUN -eq 1 && -f "$BUILD_DIR/Pong" ]]; then
    "$BUILD_DIR/Pong"
    exit 0
fi

cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DPONG_DEV=$PONG_DEV -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$BUILD_DIR" --parallel "$(nproc)"

ln -sf "$BUILD_DIR/compile_commands.json" compile_commands.json

if [[ $RUN -eq 1 ]]; then
    "$BUILD_DIR/Pong"
fi
