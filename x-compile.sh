#!/bin/bash

COL_NONE="\033[0m"
COL_GREEN="\033[0;32m"

BUILD_DIR=build
EXECUTABLE=VectemBuildTool
EXECUTABLE_DIR=bin

# Clean
if [[ "$1" == "-c" ]]; then
    printf "%b(1) Nettoyage du projet...%b\n" "$COL_GREEN" "$COL_NONE"
    rm -rf "$BUILD_DIR"
    exit 0
fi

# Build
printf "%b(1) Compilation du projet...%b\n" "$COL_GREEN" "$COL_NONE"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1

cmake .. || exit 1
cmake --build . || exit 1

# Execute
#if [[ "$1" == "-x" ]]; then
#    printf "%b(2) Exécution du programme...%b\n" "$COL_GREEN" "$COL_NONE"
#    cd ../"$EXECUTABLE_DIR" || exit 1
#    ./"$EXECUTABLE"
#fi