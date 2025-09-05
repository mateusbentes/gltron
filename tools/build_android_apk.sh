#!/usr/bin/env bash
set -euo pipefail

# Build a debug APK directly from build-android outputs (assets + libgltron.so)
# No Gradle project required.
#
# Requirements:
#  - ANDROID_SDK_ROOT must be set and contain build-tools (aapt/aapt2, d8, zipalign, apksigner)
#  - build-android/ must contain:
#      * libgltron.so (arm64-v8a) — shared library to be loaded by NativeActivity
#      * libc++_shared.so (arm64-v8a) — shared library to be loaded by libgltron.so
#      * assets files: *.sgi, *.wav, *.ftx, *.it, settings.txt, menu.txt, tron.mtl
#  - Package: org.gltron.game, App name: GLTron, minSdk:29, targetSdk:36
#
# Output:
#  - build-android/gltron.apk
#
# Usage:
#  tools/build_android_apk.sh

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
OUT_DIR="$ROOT_DIR/build-android"
ABI="${ANDROID_ABI:-arm64-v8a}"
STAGE_DIR="$ROOT_DIR/tools/staging"
mkdir -p "$STAGE_DIR"
rm -rf "$STAGE_DIR"/* 2>/dev/null || true
mkdir -p "$STAGE_DIR/lib/$ABI" || {
  err "Failed to create staging directory: $STAGE_DIR/lib/$ABI"
}
# Allow override via environment. Validate later.
PKG="${ANDROID_APP_ID:-org.gltron.game}"
APP_NAME="${ANDROID_APP_NAME:-GLTron}"
MIN_SDK=${ANDROID_MIN_SDK:-29}
TARGET_SDK=${ANDROID_TARGET_SDK:-36}
LIB_NAME="${ANDROID_LIB_NAME:-gltron}"
SO_NAME="lib${LIB_NAME}.so"
APK_OUT="$OUT_DIR/gltron.apk"
ALIGNED_APK="$STAGE_DIR/gltron-aligned.apk"

log() { echo "[build_android_apk] $*"; }
err() { echo "[build_android_apk][error] $*" >&2; exit 1; }

require_cmd() { command -v "$1" >/dev/null 2>&1 || { err "Missing required tool: $1"; }; }

# Locate Android build-tools - use version 36.0.0
BUILD_TOOLS_VERSION="36.0.0"
ANDROID_SDK_ROOT=/home/mateus/Android/Sdk
BUILD_TOOLS_DIR="$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_VERSION"

if [[ ! -d "$BUILD_TOOLS_DIR" ]]; then
  err "Build-tools version $BUILD_TOOLS_VERSION not found in $ANDROID_SDK_ROOT/build-tools"
fi

# Verify tools exist
if [[ ! -x "$BUILD_TOOLS_DIR/zipalign" || ! -x "$BUILD_TOOLS_DIR/apksigner" ]]; then
  err "zipalign or apksigner not found in $BUILD_TOOLS_DIR"
fi

# Find the latest platform version
if [[ -d "$ANDROID_SDK_ROOT/platforms" ]]; then
  HIGHEST_PLATFORM=$(ls -1 "$ANDROID_SDK_ROOT/platforms" | sed -n 's/android-\([0-9][0-9]*\)/\1/p' | sort -n | tail -n1 || true)
  if [[ -n "${HIGHEST_PLATFORM:-}" && -f "$ANDROID_SDK_ROOT/platforms/android-$HIGHEST_PLATFORM/android.jar" ]]; then
    TARGET_SDK=$HIGHEST_PLATFORM
  fi
fi

# Prefer aapt2 + d8; fallback to aapt + dx if necessary
AAPT2="$BUILD_TOOLS_DIR/aapt2"
AAPT="$BUILD_TOOLS_DIR/aapt"
D8="$BUILD_TOOLS_DIR/d8"
DX="$BUILD_TOOLS_DIR/dx"
ZIPALIGN="$BUILD_TOOLS_DIR/zipalign"
APKSIGNER="$BUILD_TOOLS_DIR/apksigner"

# Locate jarsigner
JARSIGNER=""
if [[ -x "$BUILD_TOOLS_DIR/jarsigner" ]]; then
  JARSIGNER="$BUILD_TOOLS_DIR/jarsigner"
else
  # Fallback to system jarsigner if not found in Android SDK
  JARSIGNER=$(command -v jarsigner || true)
  if [[ ! -x "$JARSIGNER" ]]; then
    err "jarsigner not found. Please install it or ensure it's in your PATH."
  fi
fi

if [[ ! -x "$ZIPALIGN" || ! -x "$APKSIGNER" ]]; then
  err "zipalign or apksigner not found in $BUILD_TOOLS_DIR"
fi

# Check shared library presence
if [[ ! -f "$OUT_DIR/$SO_NAME" ]]; then
  err "Missing $SO_NAME in $OUT_DIR. A NativeActivity APK requires a shared library."
  err "Please build your Android shared library and place it at: $OUT_DIR/$SO_NAME"
  err "Example (CMake): add_library(${LIB_NAME} SHARED ...); target ABI arm64-v8a."
fi

# Prepare staging structure (single pass)
mkdir -p "$STAGE_DIR"/{manifest,res/values,assets,lib/$ABI}

# Copy libgltron.so into the staging lib directory FIRST
log "Copying $SO_NAME to staging lib directory"
cp "$OUT_DIR/$SO_NAME" "$STAGE_DIR/lib/$ABI/" || {
  err "ERROR: Failed to copy $SO_NAME from $OUT_DIR to $STAGE_DIR/lib/$ABI/"
}
log "✅ $SO_NAME copied successfully"

# Verify the copy worked
if [[ ! -f "$STAGE_DIR/lib/$ABI/$SO_NAME" ]]; then
  err "File not found at destination: $STAGE_DIR/lib/$ABI/$SO_NAME"
fi

# Copy libc++_shared.so to the lib directory (prefer ANDROID_NDK_HOME, fallback to ANDROID_NDK)
LIBCXX_SRC=""
if [[ -n "${ANDROID_NDK_HOME:-}" && -d "${ANDROID_NDK_HOME:-}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android" ]]; then
  LIBCXX_SRC="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"
elif [[ -n "${ANDROID_NDK:-}" && -d "${ANDROID_NDK:-}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android" ]]; then
  LIBCXX_SRC="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"
fi
if [[ -n "$LIBCXX_SRC" && -f "$LIBCXX_SRC" ]]; then
  log "Copying libc++_shared.so"
  cp "$LIBCXX_SRC" "$STAGE_DIR/lib/$ABI/libc++_shared.so"
  log "✅ libc++_shared.so copied successfully"
else
  log "⚠️  Warning: libc++_shared.so not found or ANDROID_NDK_HOME not set"
fi

# Copy assets from build-android (explicitly exclude staging and other dot dirs)
log "Copying assets from $OUT_DIR"
rsync -a --delete \
  --include='*/' \
  --include='*.sgi' \
  --include='*.wav' \
  --include='*.ftx' \
  --include='*.it' \
  --include='*.obj' \
  --include='*.mtl' \
  --include='settings.txt' \
  --include='menu.txt' \
  --include='tron.mtl' \
  --exclude='.apk-stage/' \
  --exclude='.*' \
  --exclude='*' \
  "$OUT_DIR/" "$STAGE_DIR/assets/" || {
  err "Failed to copy assets"
}

# Create AndroidManifest.xml
cat > "$STAGE_DIR/AndroidManifest.xml" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="org.gltron.game"
    android:versionCode="1"
    android:versionName="1.0">
    <uses-sdk android:minSdkVersion="29" android:targetSdkVersion="36" />
    <application
        android:label="GLTron"
        android:hasCode="false"
        android:allowBackup="true"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen">
        <activity
            android:name="android.app.NativeActivity"
            android:label="GLTron"
            android:exported="true"
            android:launchMode="singleTask"
            android:screenOrientation="fullSensor"
            android:configChanges="keyboard|keyboardHidden|orientation|screenSize">
            <meta-data android:name="android.app.lib_name" android:value="gltron" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF

if [ ! -f "$STAGE_DIR/AndroidManifest.xml" ]; then
  err "ERROR: Missing AndroidManifest.xml in STAGE_DIR"
fi

# Copy additional assets from root directory
log "Copying additional assets from root directory"
cp -a "$ROOT_DIR"/menu.txt "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/settings.txt "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/xenotron.ftx "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/tron.mtl "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/t-u-low.obj "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/gltron.it "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/*.sgi "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/*.ftx "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/*.wav "$STAGE_DIR/assets/" 2>/dev/null || true

# Create strings.xml
STRINGS_TEMP=$(mktemp)
cat > "$STRINGS_TEMP" <<EOF
<resources>
    <string name="gltron">$APP_NAME</string>
</resources>
EOF

mv "$STRINGS_TEMP" "$STAGE_DIR/res/values/strings.xml" || {
  err "Failed to create strings.xml"
}

# Auto-detect highest installed Android platform if available
if [[ -d "$ANDROID_SDK_ROOT/platforms" ]]; then
  HIGHEST_PLATFORM=$(ls -1 "$ANDROID_SDK_ROOT/platforms" | sed -n 's/android-\([0-9][0-9]*\)/\1/p' | sort -n | tail -n1 || true)
  if [[ -n "${HIGHEST_PLATFORM:-}" && -f "$ANDROID_SDK_ROOT/platforms/android-$HIGHEST_PLATFORM/android.jar" ]]; then
    TARGET_SDK=$HIGHEST_PLATFORM
  fi
fi

# Final validation of staging directory
log "Validating staging directory structure..."
if [ ! -d "$STAGE_DIR/lib/$ABI" ] || [ -z "$(ls -A $STAGE_DIR/lib/$ABI)" ]; then
  err "ERROR: No native libraries in lib/$ABI"
fi

# List what's in the lib directory for debugging
log "Contents of $STAGE_DIR/lib/$ABI/:"
ls -la "$STAGE_DIR/lib/$ABI/"

log "✅ STAGE_DIR validated successfully"

# Package resources and manifest
log "Using package: $PKG"
log "Min SDK: $MIN_SDK, Target SDK: $TARGET_SDK, ABI: $ABI, LIB: $LIB_NAME"
log "Manifest: $STAGE_DIR/AndroidManifest.xml"

# Ensure temp files are cleaned on exit
cleanup() { rm -rf "$UNALIGNED_APK" "$ALIGNED_APK" 2>/dev/null || true; }
trap cleanup EXIT

UNALIGNED_APK="$(mktemp "${TMPDIR:-/tmp}/gltron-unaligned.XXXXXX.apk")"
rm -f "$APK_OUT"

if [[ -x "$AAPT" ]]; then
  log "Building APK with aapt (preferred for v1+v2 signing)"
  "$AAPT" package -f \
    -M "$STAGE_DIR/AndroidManifest.xml" \
    -S "$STAGE_DIR/res" \
    -A "$STAGE_DIR/assets" \
    -I "$ANDROID_SDK_ROOT/platforms/android-$TARGET_SDK/android.jar" \
    -F "$UNALIGNED_APK" || {
    err "Failed to package APK with aapt"
  }
  
  # Add native libraries to the APK using zip
  log "Adding native libraries to APK..."
  (cd "$STAGE_DIR" && zip -qur "$UNALIGNED_APK" lib || {
    err "Failed to add native libraries to APK"
  })
  
elif [[ -x "$AAPT2" ]]; then
  log "Building APK with aapt2 (fallback)"
  mkdir -p "$STAGE_DIR/compiled-res"
  "$AAPT2" compile -o "$STAGE_DIR/compiled-res" "$STAGE_DIR/res/values/strings.xml" || {
    err "Failed to compile resources with aapt2"
  }
  COMPILED_RES_FILES=("$STAGE_DIR"/compiled-res/*.flat)
  "$AAPT2" link -o "$UNALIGNED_APK" \
    --manifest "$STAGE_DIR/AndroidManifest.xml" \
    -I "$ANDROID_SDK_ROOT/platforms/android-$TARGET_SDK/android.jar" \
    --min-sdk-version "$MIN_SDK" \
    --target-sdk-version "$TARGET_SDK" \
    "${COMPILED_RES_FILES[@]}" || {
    err "Failed to link resources with aapt2"
  }
  
  # Add assets and native libraries to the APK
  log "Adding assets and native libraries to APK..."
  (cd "$STAGE_DIR" && zip -qur "$UNALIGNED_APK" assets lib || {
    err "Failed to add assets and libs to APK"
  })
else
  err "Neither aapt nor aapt2 found in $BUILD_TOOLS_DIR"
fi

# Verify the unaligned APK was created and contains the library
if [[ ! -f "$UNALIGNED_APK" ]]; then
  err "Failed to create unaligned APK"
fi

# Debug: Check if the library is in the unaligned APK
log "Checking contents of unaligned APK..."
unzip -l "$UNALIGNED_APK" | grep -E "(lib/|\.so)" || {
  log "⚠️  Warning: No native libraries found in APK!"
  log "APK contents:"
  unzip -l "$UNALIGNED_APK"
}

# Align and sign
ALIGNED_APK="$(mktemp "${TMPDIR:-/tmp}/gltron-aligned.XXXXXX.apk")"
"$ZIPALIGN" -f 4 "$UNALIGNED_APK" "$ALIGNED_APK" || {
  err "Failed to align APK"
}

# Verify the aligned APK was created
if [[ ! -f "$ALIGNED_APK" ]]; then
  err "Failed to create aligned APK"
fi

# Signing: use debug keystore with both v1 and v2 schemes (apksigner only)
log "Signing APK with debug keystore (v1 and v2 schemes)"
KEYSTORE="$HOME/.android/debug.keystore"
KEYALIAS="androiddebugkey"
KEYPASS="android"

# Ensure the keystore exists
if [[ ! -f "$KEYSTORE" ]]; then
  err "Debug keystore not found at $KEYSTORE"
fi

# Verify the keystore can be accessed
if ! keytool -list -keystore "$KEYSTORE" -storepass "$KEYPASS" >/dev/null 2>&1; then
  err "Cannot access debug keystore. Check passphrase or keystore integrity."
fi

"$APKSIGNER" sign \
  --ks "$KEYSTORE" \
  --ks-pass pass:"$KEYPASS" \
  --key-pass pass:"$KEYPASS" \
  --v1-signing-enabled true \
  --v2-signing-enabled true \
  --v3-signing-enabled false \
  --out "$APK_OUT" \
  "$ALIGNED_APK" || {
  err "Failed to sign APK with apksigner (v1+v2)"
}

# Verify the signing (require v2, warn if v1 is false)
VERIFY_OUT=$("$APKSIGNER" verify --verbose "$APK_OUT")
if ! echo "$VERIFY_OUT" | grep -q "Verified using v2 scheme (APK Signature Scheme v2): true"; then
  echo "$VERIFY_OUT"
  err "Failed to sign APK with v2 scheme"
fi
if ! echo "$VERIFY_OUT" | grep -q "Verified using v1 scheme (JAR signing): true"; then
  log "Warning: v1 (JAR) signing is false. This is acceptable for minSdk $MIN_SDK (v2 is present)."
fi

# Final verification - check if the library is in the final APK
log "Final verification - checking APK contents..."
unzip -l "$APK_OUT" | grep -E "(lib/|\.so)" && {
  log "✅ Native libraries found in final APK!"
} || {
  log "❌ ERROR: No native libraries found in final APK!"
  log "Final APK contents:"
  unzip -l "$APK_OUT"
}

log "APK built and signed (v2 verified): $APK_OUT"
echo "$APK_OUT"
