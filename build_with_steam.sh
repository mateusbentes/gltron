#!/bin/bash

# Build GLTron with Steam multiplayer support

echo "Building GLTron with Steam Multiplayer Support..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with Steam support
echo "Configuring CMake with Steamworks..."
cmake .. -DUSE_STEAMWORKS=ON -DUSE_SOUND=ON

# Build
echo "Building..."
make -j$(nproc)

# Copy Steam files
echo "Setting up Steam files..."
cp ../steam_appid.txt .
cp ../steamworks/sdk/redistributable_bin/linux64/libsteam_api.so . 2>/dev/null || \
cp ../steamworks/sdk/redistributable_bin/linux32/libsteam_api.so . 2>/dev/null

echo "Build complete!"
echo ""
echo "To run GLTron with multiplayer:"
echo "1. Make sure Steam is running"
echo "2. Run: ./gltron"
echo "3. Go to Multiplayer menu to create or join games"
echo ""
echo "Note: Using App ID 480 (Spacewar) for testing"
