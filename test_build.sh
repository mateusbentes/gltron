#!/bin/bash

echo "Testing GLTron menu fixes..."

# Create a simple test compilation
echo "Compiling minimal menu test..."

# Test compile the minimal menu test
gcc -I. -I./src -I./src/include \
    test_menu_minimal.c \
    -o test_menu \
    $(pkg-config --cflags --libs sdl2 gl) 2>&1

if [ $? -eq 0 ]; then
    echo "✅ Menu test compiles successfully!"
    echo "Run './test_menu' to test the menu functionality"
else
    echo "❌ Compilation failed. Check the errors above."
    exit 1
fi

echo "Build test completed successfully!"
