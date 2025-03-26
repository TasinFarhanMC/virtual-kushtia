#!/bin/bash

set -e # Exit on error

DIR=$(dirname "$(realpath "$0")")
cd "$DIR"

# Determine build type (default: debug)
BUILD_TYPE="debug"
BIN_NAME="server"
RERUN_CMAKE=0

if [[ -n "$1" ]]; then
  case "$1" in
  d | 0) BUILD_TYPE="debug" ;;
  r) BUILD_TYPE="release" ;;
  t) BUILD_TYPE="test" ;;
  *) RERUN_CMAKE=1 ;;
  esac
fi
[[ -n "$1" ]] && shift

# Rerun CMake if required
if [[ "$RERUN_CMAKE" -eq 1 || (-n "$1" && "$1" != "0") ]]; then
  cmake --preset "$BUILD_TYPE"
fi

# Build the selected target
cmake --build --preset "$BUILD_TYPE" -- -j 6

# Run tests if -t flag was used
if [[ "$BUILD_TYPE" == "test" ]]; then
  cd build/debug
  ctest -V
  exit 0
fi

# Function to monitor web directory changes (only in debug mode)
start_monitor_web() {
  SRC_DIR="$DIR/web"
  DEST_DIR="$DIR/dist/web"
  SIG_FILE="$DEST_DIR/update.sig"

  # Ensure inotifywait is installed
  if ! command -v inotifywait &>/dev/null; then
    echo "Error: inotifywait (from inotify-tools) is not installed."
    exit 1
  fi

  echo "Monitoring changes in $SRC_DIR and copying to $DEST_DIR..."

  # Start monitoring for file modifications, creations, and moves
  inotifywait -m -r -e modify,create,move "$SRC_DIR" --format "%w%f" | while read -r FILE; do
    # Ensure the destination directory exists
    mkdir -p "$DEST_DIR"

    # Preserve relative path structure
    REL_PATH="${FILE#"$SRC_DIR"/}"
    DEST_FILE="$DEST_DIR/$REL_PATH"

    # Create parent directories if necessary
    mkdir -p "$(dirname "$DEST_FILE")"

    # Copy the modified or new file
    cp "$FILE" "$DEST_FILE"

    # Create or update the signal file with a timestamp
    touch "$SIG_FILE"

    echo "Copied: $FILE -> $DEST_FILE"
    echo "Updated signal file: $SIG_FILE"
  done
}

# Run background monitoring script only in debug mode
if [[ "$BUILD_TYPE" == "debug" ]]; then
  start_monitor_web &
  MONITOR_PID=$!

  # Cleanup function to handle SIGINT and SIGTERM
  cleanup() {
    kill "$MONITOR_PID" 2>/dev/null
    wait "$MONITOR_PID" 2>/dev/null || true
    exit 0 # Always exit with 0
  }

  trap cleanup SIGINT SIGTERM
fi

# Run the selected binary and pass all remaining arguments
cd dist
./"$BIN_NAME" "$@"

# Ensure cleanup runs when the main program exits (only if in debug mode)
[[ "$BUILD_TYPE" == "debug" ]] && cleanup
