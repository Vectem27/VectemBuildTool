#!/bin/bash

EXECUTABLE=VectemBuildTool
EXECUTABLE_DIR=bin

cd ./"$EXECUTABLE_DIR" || exit 1
./"$EXECUTABLE" "$@"