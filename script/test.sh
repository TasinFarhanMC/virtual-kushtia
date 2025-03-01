#!/bin/bash

set -e # Exit on error

DIR=$(dirname "$(dirname "$(realpath "$0")")")
cd "$DIR"

# Run CMake if build directory is missing or first arg is not "0"
[[ ! -d "$DIR/build" || (-n "$1" && "$1" != "0") ]] && cmake --preset debug

# Build the project
cmake --build --preset debug -- -j "$(nproc)"

# Change to dist directory
cd dist

# Remove first argument if it's "0"
[[ "$1" == "0" ]] && shift

# Run the program with remaining arguments
./virtual-kushtia "$@"
