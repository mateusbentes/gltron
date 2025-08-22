#!/usr/bin/env bash
set -euo pipefail

# This script demonstrates configuring desktop and Android builds.
# Usage:
#   ./tools/configure_examples.sh [desktop|android|both]
# Prereqs:
#   - For desktop: a working compiler and OpenGL/GLUT dev packages.
#   - For android: ANDROID_NDK environment variable must be set.

MODE=${1:-both}
ROOT_DIR=$(cd "$(dirname "$0")/.." && pwd)

run_desktop() {
  echo "==> Configuring desktop build (host toolchain)"
  rm -rf "$ROOT_DIR/build-pc-config" && mkdir -p "$ROOT_DIR/build-pc-config"
  cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build-pc-config" \
    -DUSE_SOUND=ON \
    -DSTRICT_ARCH_CHECK=OFF
  echo "Desktop configure done in build-pc-config"
}

run_android() {
  : "${ANDROID_NDK:?ANDROID_NDK is not set}"
  echo "==> Configuring Android build (ABI arm64-v8a)"
  rm -rf "$ROOT_DIR/build-android-config" && mkdir -p "$ROOT_DIR/build-android-config"
  cmake -S "$ROOT_DIR" -B "$ROOT_DIR/build-android-config" \
    -DANDROID=ON \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=29 \
    -DUSE_SOUND=ON
  echo "Android configure done in build-android-config"
}

case "$MODE" in
  desktop) run_desktop ;;
  android) run_android ;;
  both) run_desktop; run_android ;;
  *) echo "Unknown mode: $MODE"; exit 1 ;;
esac
