#!/bin/bash

# Define the directory path
BASE_DIR="/opt/lampp/htdocs"
TARGET_DIR_NAME="bolprocessor"
TARGET_DIR=""

# Function to find the target directory
find_target_directory() {
    TARGET_DIR=$(find "$BASE_DIR" -type d -name "$TARGET_DIR_NAME" 2>/dev/null | head -n 1)
    if [ -z "$TARGET_DIR" ]; then
        echo "Error: $TARGET_DIR_NAME not found in $BASE_DIR."
        exit 1
    fi
}

# Find the target directory
find_target_directory

# Change ownership to 'daemon' and permissions to '775' recursively
echo "Changing owner to 'daemon' and permissions to '775' for all files and directories in $TARGET_DIR..."

sudo chown -R daemon:daemon "$TARGET_DIR"
sudo chmod -R 775 "$TARGET_DIR"

echo "Completed changing owner to 'daemon' and permissions to '775' for all files and directories in $TARGET_DIR."
