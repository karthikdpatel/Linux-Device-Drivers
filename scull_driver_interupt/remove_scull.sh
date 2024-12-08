#!/bin/bash

# Script to remove the scull kernel module and delete its device node

# Exit immediately if a command exits with a non-zero status
set -e

MODULE_NAME="scull"
DEVICE_NAME="/dev/scull"

echo "Starting the removal process for the $MODULE_NAME module."

# Function to handle errors
error_handler() {
    echo "An error occurred. Exiting the script."
    exit 1
}

# Trap errors
trap 'error_handler' ERR

# Check if the user has sudo privileges
if ! sudo -v; then
    echo "This script requires sudo privileges. Exiting."
    exit 1
fi

# Function to check if the module is loaded
is_module_loaded() {
    lsmod | grep "^$MODULE_NAME\s" &> /dev/null
}

# Function to remove the module
remove_module() {
    echo "Removing module $MODULE_NAME..."
    sudo rmmod $MODULE_NAME
    echo "Module $MODULE_NAME removed successfully."
}

# Function to check if the device node exists
device_node_exists() {
    [ -e "$DEVICE_NAME" ]
}

# Function to remove the device node
remove_device_node() {
    echo "Deleting device node $DEVICE_NAME..."
    sudo rm $DEVICE_NAME
    echo "Device node $DEVICE_NAME deleted successfully."
}

# Check if the module is loaded
if is_module_loaded; then
    remove_module
else
    echo "Module $MODULE_NAME is not loaded. Skipping module removal."
fi

# Check if the device node exists
if device_node_exists; then
    remove_device_node
else
    echo "Device node $DEVICE_NAME does not exist. Skipping device node removal."
fi

echo "Removal process completed successfully."

