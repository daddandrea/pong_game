#!/bin/sh
# Usage: update.sh <archive_path> <install_dir> <executable_path>
ARCHIVE="$1"
INSTALL_DIR="$2"
EXECUTABLE="$3"

sleep 1
tar -xzf "$ARCHIVE" -C "$INSTALL_DIR" --strip-components=1
rm "$ARCHIVE"
"$EXECUTABLE" &
rm -- "$0"
