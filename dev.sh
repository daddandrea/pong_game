#!/usr/bin/env bash
set -e

COMMAND=$1
FLAG=$2

usage() {
    echo "Usage: ./dev.sh <command> [--debug|--release]"
    echo ""
    echo "  Commands:"
    echo "    build --debug      Debug build with PONG_DEV=ON  →  build/debug/"
    echo "    build --release    Release build                 →  build/release/"
    echo "    run   --debug      Run debug binary (build first if needed)"
    echo "    run   --release    Run release binary (build first if needed)"
    echo "    clean              Delete all of build/ and the symlink"
    echo "    clean --debug      Delete only build/debug/"
    echo "    clean --release    Delete only build/release/"
    echo ""
    echo "  Other:"
    echo "    -h                 Show this help"
}

require_build_flag() {
    if [[ "$FLAG" != "--debug" && "$FLAG" != "--release" ]]; then
        echo "Error: '$COMMAND' requires --debug or --release"
        echo ""
        usage
        exit 1
    fi
}

case $COMMAND in
    -h)
        usage
        exit 0
        ;;

    build)
        require_build_flag

        if [[ "$FLAG" == "--debug" ]]; then
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

        if [[ "$FLAG" == "--debug" ]]; then
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
        elif [[ "$FLAG" == "--debug" ]]; then
            rm -rf build/debug
            [[ -L compile_commands.json && "$(readlink compile_commands.json)" == "build/debug/compile_commands.json" ]] && rm -f compile_commands.json
            echo "Cleaned build/debug"
        elif [[ "$FLAG" == "--release" ]]; then
            rm -rf build/release
            [[ -L compile_commands.json && "$(readlink compile_commands.json)" == "build/release/compile_commands.json" ]] && rm -f compile_commands.json
            echo "Cleaned build/release"
        else
            echo "Error: unknown flag '$FLAG'"
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
