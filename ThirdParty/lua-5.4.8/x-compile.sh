#!/bin/bash

COL_NONE="\033[0m"
COL_GREEN="\033[0;32m"

BUILD_DIR=build
OUTPUT_DIR=bin

# Clean
if [[ "$1" == "-c" ]]; then
    printf "%bClean lua build...%b\n" "$COL_GREEN" "$COL_NONE"
    rm -rf "$BUILD_DIR"
    exit 0
fi

# Build
printf "%bCompile lua...%b\n" "$COL_GREEN" "$COL_NONE"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

cmake .. || exit 1
cmake --build . || exit 1