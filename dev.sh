#!/usr/bin/env bash
set -e

COMMAND=$1
FLAG=$2

readonly debug_flag = "--debug"
readonly release_flag = "--release"

usage() {
    echo "Usage: ./dev.sh <command> [$debug_flag|$release_flag]"
    echo ""
    echo "  Commands:"
    echo "    build $debug_flag      Debug build with PONG_DEV=ON  →  build/debug/"
    echo "    build $release_flag    Release build                 →  build/release/"
    echo "    run   $debug_flag      Run debug binary (build first if needed)"
    echo "    run   $release_flag    Run release binary (build first if needed)"
    echo "    clean              Delete all of build/ and the symlink"
    echo "    clean $debug_flag      Delete only build/debug/"
    echo "    clean $release_flag    Delete only build/release/"
    echo ""
    echo "  Other:"
    echo "    -h                 Show this help"
    return 0
}

require_build_flag() {
    if [[ "$FLAG" != $debug_flag && "$FLAG" != $release_flag ]]; then
        echo "Error: '$COMMAND' requires $debug_flag or $release_flag" >&2
        echo ""
        usage
        exit 1
    fi
    return 0
}

case $COMMAND in
    -h)
        usage
        exit 0
        ;;

    build)
        require_build_flag

        if [[ "$FLAG" == "${debug_flag}" ]]; then
            BUILD_TYPE=Debug; PONG_DEV=ON
        else
            BUILD_TYPE=Release; PONG_DEV=OFF
        fi

        BUILD_DIR=build/$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

        cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DPONG_DEV=$PONG_DEV -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        cmake --build "$BUILD_DIR" --parallel "$(nproc)"
        ln -sf "$BUILD_DIR/compile_commands.json" compile_commands.json
        ;;

    run)
        require_build_flag

        if [[ "$FLAG" == "${debug_flag}" ]]; then
            BUILD_TYPE=Debug; PONG_DEV=ON
        else
            BUILD_TYPE=Release; PONG_DEV=OFF
        fi

        BUILD_DIR=build/$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

        if [[ ! -f "$BUILD_DIR/Pong" ]]; then
            cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DPONG_DEV=$PONG_DEV -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
            cmake --build "$BUILD_DIR" --parallel "$(nproc)"
            ln -sf "$BUILD_DIR/compile_commands.json" compile_commands.json
        fi

        "$BUILD_DIR/Pong"
        ;;

    clean)
        if [[ -z "$FLAG" ]]; then
            rm -rf build
            rm -f compile_commands.json
            echo "Cleaned all"
        elif [[ "$FLAG" == "${debug_flag}" ]]; then
            rm -rf build/debug
            [[ -L compile_commands.json && "$(readlink compile_commands.json)" == "build/debug/compile_commands.json" ]] && rm -f compile_commands.json
            echo "Cleaned build/debug"
        elif [[ "$FLAG" == "${release_flag}" ]]; then
            rm -rf build/release
            [[ -L compile_commands.json && "$(readlink compile_commands.json)" == "build/release/compile_commands.json" ]] && rm -f compile_commands.json
            echo "Cleaned build/release"
        else
            echo "Error: unknown flag '$FLAG'" >&2
            echo ""
            usage
            exit 1
        fi
        ;;

    *)
        usage
        exit 1
        ;;
esac
