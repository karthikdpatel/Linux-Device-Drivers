#!/bin/bash

# Script to build and install the scull kernel module

# Exit immediately if a command exits with a non-zero status
set -e

MODULE_NAME="scull"
DEVICE_NAME="/dev/scull"

echo "Starting the build and installation process for the $MODULE_NAME module."

# Check if the script is run as root
if [ "$EUID" -ne 0 ]; then
    echo "Please run this script with sudo or as root."
    exit 1
fi

# Clean previous builds
echo "Cleaning previous builds..."
make clean

# Build the module
echo "Building the module..."
make

# Check if the module is already loaded
if lsmod | grep "$MODULE_NAME" &> /dev/null ; then
    echo "Module $MODULE_NAME is already loaded. Removing it first."
    rmmod $MODULE_NAME
    echo "Module $MODULE_NAME removed."
fi

# Insert the module into the kernel
echo "Inserting the module into the kernel..."
insmod ${MODULE_NAME}.ko

# Verify the module was inserted successfully
if lsmod | grep "$MODULE_NAME" &> /dev/null ; then
    echo "Module $MODULE_NAME inserted successfully."
else
    echo "Failed to insert module $MODULE_NAME."
    exit 1
fi

# Retrieve the major number from /proc/devices
MAJOR_NUM=$(awk "/$MODULE_NAME/ {print \$1}" /proc/devices)
if [ -z "$MAJOR_NUM" ]; then
    echo "Could not find the major number for $MODULE_NAME."
    exit 1
fi
echo "Allocated Major Number: $MAJOR_NUM"

# Create the device node if it doesn't exist
if [ ! -e "$DEVICE_NAME" ]; then
    echo "Creating device node $DEVICE_NAME with major $MAJOR_NUM and minor 0..."
    mknod $DEVICE_NAME c $MAJOR_NUM 0
    echo "Device node $DEVICE_NAME created."
else
    echo "Device node $DEVICE_NAME already exists."
fi

# Set permissions for the device node
echo "Setting permissions for $DEVICE_NAME..."
chmod 666 $DEVICE_NAME

echo "Build and installation process completed successfully."

