#!/bin/bash

# Define the directory path
TARGET_DIR_NAME="bolprocessor/php"
TARGET_DIR=""

# Extract the top-level directory (first component of TARGET_DIR_NAME)
TOP_LEVEL_DIR=$(echo "$TARGET_DIR_NAME" | cut -d'/' -f1)

# Function to find the top-level target directory
find_target_directory() {
    TARGET_DIR=$(find / -type d -name "$TOP_LEVEL_DIR" 2>/dev/null | head -n 1)
    if [ -z "$TARGET_DIR" ]; then
        echo "Error: $TOP_LEVEL_DIR not found in the system."
        exit 1
    fi
}

# Find the target directory 
find_target_directory

# Make sure that user 'daemon' belongs to group 'linuxlite'
usermod -aG linuxlite daemon

# Change ownership to 'daemon' and permissions to '775' recursively
echo "Changing owner/group to 'daemon:linuxlite' and permissions to '775' for all files and directories in $TARGET_DIR..."

sudo chown -R daemon:linuxlite "$TARGET_DIR"
sudo chmod -R 775 "$TARGET_DIR"

echo "Completed changing owner and permissions for all files and directories in $TARGET_DIR."
