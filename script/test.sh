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

# Define source and destination for monitoring
SRC_DIR="$DIR/web"
DEST_DIR="$DIR/dist/web"

# Start the file monitor script in the background
: >"$DIR/build/monitor_web.log" && "$DIR/script/monitor_web.sh" "$SRC_DIR" "$DEST_DIR" >"$DIR/build/monitor_web.log" 2>&1 &

# Store the PID of the monitoring process
MONITOR_PID=$!

# Run the main program and wait for it to finish
./virtual-kushtia "$@"

# Kill the monitoring script when the main program exits
kill $MONITOR_PID 2>/dev/null
wait $MONITOR_PID 2>/dev/null || true # Ensure cleanup
