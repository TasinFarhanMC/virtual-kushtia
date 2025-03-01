#!/bin/bash

# Source and destination directories
SRC_DIR="$1"
DEST_DIR="$2"
SIG_FILE="$DEST_DIR/update.sig"

# Ensure both directories are provided
if [[ -z "$SRC_DIR" || -z "$DEST_DIR" ]]; then
  echo "Usage: $0 <source_directory> <destination_directory>"
  exit 1
fi

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
