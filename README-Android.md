Building for Android and Desktop
================================

This document describes how to build GLTron for Android using the Android NDK and for desktop using your host toolchain.

Prerequisites
-------------
- CMake 3.10+
- Desktop build: a C/C++ compiler, OpenGL and GLUT dev packages (e.g. Debian/Ubuntu: sudo apt-get install build-essential freeglut3-dev mesa-common-dev)
- Android build: Android NDK installed and ANDROID_NDK environment variable set

Quick examples
--------------
A helper script is provided to configure both builds:

  ./tools/configure_examples.sh both

Desktop build (host-native)
---------------------------
  mkdir -p build-pc && cd build-pc
  cmake -DUSE_SOUND=ON -DSTRICT_ARCH_CHECK=OFF ..
  cmake --build .

Notes:
- STRICT_ARCH_CHECK, when ON, verifies that desktop third-party sound libraries are x86_64; default is OFF to avoid failures on non-x86 hosts.
- CMake tries find_package(OpenGL/GLUT) first and falls back to common x86_64 paths when necessary.

Android build (arm64-v8a)
-------------------------
  export ANDROID_NDK=/path/to/android/ndk
  mkdir -p build-android && cd build-android
  cmake -DANDROID=ON -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=29 -DUSE_SOUND=ON ..
  cmake --build .

Notes:
- Prebuilt libopenmpt from android-dependencies/<ABI>/prefix is used when available; otherwise, the project attempts to build it.
- Android-specific toolchain and libraries are configured only when -DANDROID=ON is passed to CMake.
