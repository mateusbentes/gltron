#!/bin/bash

# Exit on error
set -e

# Check if ANDROID_NDK environment variable is set
if [ -z "$ANDROID_NDK" ]; then
    echo "Error: ANDROID_NDK environment variable is not set."
    echo "Please set it to the path of your Android NDK installation."
    exit 1
fi

# Create build directory
mkdir -p build-android
cd build-android

# Configure with CMake
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../android/android.toolchain.cmake \
    -DBUILD_ANDROID=ON \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -- -j$(nproc)

echo "Build completed successfully!"