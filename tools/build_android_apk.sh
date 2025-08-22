#!/usr/bin/env bash
set -euo pipefail

# Build a debug APK directly from build-android outputs (assets + libgltron.so)
# No Gradle project required.
#
# Requirements:
#  - ANDROID_SDK_ROOT must be set and contain build-tools (aapt/aapt2, d8, zipalign, apksigner)
#  - build-android/ must contain:
#      * libgltron.so (arm64-v8a) — shared library to be loaded by NativeActivity
#      * assets files: *.sgi, *.wav, *.ftx, *.it, settings.txt, menu.txt, tron.mtl
#  - Package: com.gltron, App name: GLTron, minSdk:21, targetSdk:33
#
# Output:
#  - build-android/gltron-debug.apk
#
# Usage:
#  tools/build_android_apk.sh

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
OUT_DIR="$ROOT_DIR/build-android"
STAGE_DIR="$OUT_DIR/.apk-stage"
PKG="com.gltron"
APP_NAME="GLTron"
ABI="arm64-v8a"
MIN_SDK=21
TARGET_SDK=33
LIB_NAME="gltron"
SO_NAME="lib${LIB_NAME}.so"
APK_OUT="$OUT_DIR/gltron-debug.apk"

log() { echo "[build_android_apk] $*"; }
err() { echo "[build_android_apk][error] $*" >&2; }

require_cmd() { command -v "$1" >/dev/null 2>&1 || { err "Missing required tool: $1"; exit 1; }; }

# Locate Android build-tools
if [[ -z "${ANDROID_SDK_ROOT:-}" ]]; then
  err "ANDROID_SDK_ROOT is not set. Please set it to your Android SDK path."; exit 1;
fi
BUILD_TOOLS_DIR=""
if [[ -d "$ANDROID_SDK_ROOT/build-tools" ]]; then
  BUILD_TOOLS_DIR=$(ls -1 "$ANDROID_SDK_ROOT/build-tools" | sort -V | tail -n1)
  BUILD_TOOLS_DIR="$ANDROID_SDK_ROOT/build-tools/$BUILD_TOOLS_DIR"
fi
if [[ -z "$BUILD_TOOLS_DIR" || ! -d "$BUILD_TOOLS_DIR" ]]; then
  err "Could not locate build-tools under $ANDROID_SDK_ROOT/build-tools"; exit 1;
fi

# Prefer aapt2 + d8; fallback to aapt + dx if necessary
AAPT2="$BUILD_TOOLS_DIR/aapt2"
AAPT="$BUILD_TOOLS_DIR/aapt"
D8="$BUILD_TOOLS_DIR/d8"
DX="$BUILD_TOOLS_DIR/dx"
ZIPALIGN="$BUILD_TOOLS_DIR/zipalign"
APKSIGNER="$BUILD_TOOLS_DIR/apksigner"

if [[ ! -x "$ZIPALIGN" || ! -x "$APKSIGNER" ]]; then
  err "zipalign or apksigner not found in $BUILD_TOOLS_DIR"; exit 1;
fi

# Check shared library presence
if [[ ! -f "$OUT_DIR/$SO_NAME" ]]; then
  err "Missing $SO_NAME in $OUT_DIR. A NativeActivity APK requires a shared library."
  err "Please build your Android shared library and place it at: $OUT_DIR/$SO_NAME"
  err "Example (CMake): add_library(${LIB_NAME} SHARED ...); target ABI arm64-v8a."
  exit 1
fi

# Prepare staging structure
rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR"/{manifest,res/values,assets,lib/$ABI}

# Copy assets from build-android
log "Copying assets from $OUT_DIR"
rsync -a \
  --include='*/' \
  --include='*.sgi' \
  --include='*.wav' \
  --include='*.ftx' \
  --include='*.it' \
  --include='settings.txt' \
  --include='menu.txt' \
  --include='tron.mtl' \
  --exclude='*' \
  "$OUT_DIR/" "$STAGE_DIR/assets/"

# Copy native library
cp "$OUT_DIR/$SO_NAME" "$STAGE_DIR/lib/$ABI/$SO_NAME"

# Minimal manifest for NativeActivity
cat > "$STAGE_DIR/manifest/AndroidManifest.xml" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="$PKG"
    android:versionCode="1"
    android:versionName="1.0">
    <uses-sdk android:minSdkVersion="$MIN_SDK" android:targetSdkVersion="$TARGET_SDK"/>
    <application android:label="@string/app_name" android:hasCode="true"
        android:allowBackup="true"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen">
        <activity android:name="android.app.NativeActivity"
            android:label="@string/app_name"
            android:exported="true"
            android:configChanges="keyboard|keyboardHidden|orientation|screenSize">
            <meta-data android:name="android.app.lib_name" android:value="$LIB_NAME" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF

# Strings resource
cat > "$STAGE_DIR/res/values/strings.xml" <<EOF
<resources>
    <string name="app_name">$APP_NAME</string>
</resources>
EOF

# No Java/DEX needed for NativeActivity-only APK; skip classes.dex generation

# Auto-detect highest installed Android platform if available
if [[ -d "$ANDROID_SDK_ROOT/platforms" ]]; then
  HIGHEST_PLATFORM=$(ls -1 "$ANDROID_SDK_ROOT/platforms" | sed -n 's/android-\([0-9][0-9]*\)/\1/p' | sort -n | tail -n1 || true)
  if [[ -n "${HIGHEST_PLATFORM:-}" && -f "$ANDROID_SDK_ROOT/platforms/android-$HIGHEST_PLATFORM/android.jar" ]]; then
    TARGET_SDK=$HIGHEST_PLATFORM
  fi
fi

# Package resources and manifest
UNALIGNED_APK="$OUT_DIR/tmp-unaligned.apk"
rm -f "$UNALIGNED_APK" "$APK_OUT"

if [[ -x "$AAPT2" ]]; then
  log "Building APK with aapt2"
  mkdir -p "$STAGE_DIR/compiled-res"
  "$AAPT2" compile -o "$STAGE_DIR/compiled-res" "$STAGE_DIR/res/values/strings.xml"
  # Collect all compiled .flat files
  COMPILED_RES_FILES=("$STAGE_DIR"/compiled-res/*.flat)
  "$AAPT2" link -o "$UNALIGNED_APK" \
    --manifest "$STAGE_DIR/manifest/AndroidManifest.xml" \
    -I "$ANDROID_SDK_ROOT/platforms/android-$TARGET_SDK/android.jar" \
    --min-sdk-version $MIN_SDK \
    --target-sdk-version $TARGET_SDK \
    "${COMPILED_RES_FILES[@]}"
  # Add assets and native libs to the APK
  (cd "$STAGE_DIR" && zip -qur "$UNALIGNED_APK" assets lib || true)
else
  if [[ ! -x "$AAPT" ]]; then
    err "Neither aapt2 nor aapt found in $BUILD_TOOLS_DIR"; exit 1;
  fi
  log "Building APK with aapt (legacy)"
  "$AAPT" package -f -M "$STAGE_DIR/manifest/AndroidManifest.xml" \
    -S "$STAGE_DIR/res" -I "$ANDROID_SDK_ROOT/platforms/android-$TARGET_SDK/android.jar" \
    -F "$UNALIGNED_APK" "$STAGE_DIR"
fi

# Align and sign
ALIGNED_APK="$OUT_DIR/tmp-aligned.apk"
"$ZIPALIGN" -f 4 "$UNALIGNED_APK" "$ALIGNED_APK"

# Debug keystore
KEYSTORE="$HOME/.android/debug.keystore"
KEYALIAS="androiddebugkey"
KEYPASS="android"
if [[ ! -f "$KEYSTORE" ]]; then
  log "Generating debug keystore at $KEYSTORE"
  mkdir -p "$(dirname "$KEYSTORE")"
  keytool -genkey -v -keystore "$KEYSTORE" -storepass "$KEYPASS" -alias "$KEYALIAS" -keypass "$KEYPASS" -keyalg RSA -keysize 2048 -validity 10000 -dname "CN=Android Debug,O=Android,C=US"
fi
"$APKSIGNER" sign --ks "$KEYSTORE" --ks-pass pass:$KEYPASS --key-pass pass:$KEYPASS --out "$APK_OUT" "$ALIGNED_APK"

log "APK built: $APK_OUT"
echo "$APK_OUT"
