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
#  - Package: org.gltron.game, App name: GLTron, minSdk:29, targetSdk:35
#
# Output:
#  - build-android/gltron.apk
#
# Usage:
#  tools/build_android_apk.sh

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
OUT_DIR="$ROOT_DIR/build-android"
STAGE_DIR="$(mktemp -d "${TMPDIR:-/tmp}/gltron-apk.XXXXXX")"
# Allow override via environment. Validate later.
PKG="${ANDROID_APP_ID:-org.gltron.game}"
APP_NAME="${ANDROID_APP_NAME:-GLTron}"
ABI="${ANDROID_ABI:-arm64-v8a}"
MIN_SDK=${ANDROID_MIN_SDK:-29}
TARGET_SDK=${ANDROID_TARGET_SDK:-35}
LIB_NAME="${ANDROID_LIB_NAME:-gltron}"
SO_NAME="lib${LIB_NAME}.so"
APK_OUT="$OUT_DIR/gltron.apk"

log() { echo "[build_android_apk] $*"; }
err() { echo "[build_android_apk][error] $*" >&2; exit 1; }

require_cmd() { command -v "$1" >/dev/null 2>&1 || { err "Missing required tool: $1"; }; }

# Locate Android build-tools - use version 35.0.1
BUILD_TOOLS_VERSION="35.0.1"
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

# Copy native libraries and minimal classes.dex (to satisfy some v1 signing expectations)
cp "$OUT_DIR/$SO_NAME" "$STAGE_DIR/lib/$ABI/$SO_NAME" || {
  err "Failed to copy native library"
}

# Copy libc++_shared.so to the lib directory (prefer ANDROID_NDK_HOME, fallback to ANDROID_NDK)
LIBCXX_SRC=""
if [[ -n "${ANDROID_NDK_HOME:-}" && -d "${ANDROID_NDK_HOME:-}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android" ]]; then
  LIBCXX_SRC="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"
elif [[ -n "${ANDROID_NDK:-}" && -d "${ANDROID_NDK:-}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android" ]]; then
  LIBCXX_SRC="$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so"
fi
if [[ -n "$LIBCXX_SRC" && -f "$LIBCXX_SRC" ]]; then
  cp "$LIBCXX_SRC" "$STAGE_DIR/lib/$ABI/libc++_shared.so"
else
  err "Failed to copy libc++_shared.so (source not found). Ensure ANDROID_NDK_HOME points to a valid NDK."
fi

# Compile minimal Java helper (UiHelpers) into classes.dex
JAVA_SRC_DIR="$ROOT_DIR/tools/java"
if [[ -d "$JAVA_SRC_DIR" ]]; then
  echo "Compiling Java helper sources..."
  mkdir -p "$STAGE_DIR/java-classes"
  # Find android.jar (API 29+) for compilation
  ANDROID_JAR=$(python3 - <<'PY'
import os
candidates=[
  '/usr/lib/android-sdk/platforms/android-35/android.jar',
  '/usr/lib/android-sdk/platforms/android-34/android.jar',
  '/opt/android-sdk/platforms/android-35/android.jar',
  os.path.expanduser('~/Android/Sdk/platforms/android-35/android.jar'),
  os.path.expanduser('~/Android/Sdk/platforms/android-34/android.jar'),
]
for c in candidates:
  if os.path.isfile(c):
    print(c)
    break
PY
)
  if [[ -z "$ANDROID_JAR" ]]; then
    echo "Warning: android.jar not found; skipping Java compile"
  else
    find "$JAVA_SRC_DIR" -name "*.java" > "$STAGE_DIR/java-sources.list"
    if [[ -s "$STAGE_DIR/java-sources.list" ]]; then
      javac -source 1.8 -target 1.8 -bootclasspath "$ANDROID_JAR" -classpath "$ANDROID_JAR" -d "$STAGE_DIR/java-classes" @"$STAGE_DIR/java-sources.list"
      if [[ $? -eq 0 ]]; then
        echo "Verifying compiled classes contain UiHelpers..."
        if ! find "$STAGE_DIR/java-classes" -type f -name "UiHelpers.class" | grep -q UiHelpers.class; then
          echo "ERROR: UiHelpers.class not found after javac; check package/path"
          exit 1
        fi
        echo "Packaging classes into jar for inspection..."
        (cd "$STAGE_DIR/java-classes" && jar cf "$STAGE_DIR/java-classes.jar" .)
        if ! jar tf "$STAGE_DIR/java-classes.jar" | grep -q "org/gltron/game/UiHelpers.class"; then
          echo "ERROR: org/gltron/game/UiHelpers.class missing in jar; check Java package declaration"
          jar tf "$STAGE_DIR/java-classes.jar" | sed -n '1,200p'
          exit 1
        fi
        echo "Converting Java classes to DEX..."
        # Locate d8 from Android SDK build-tools if not on PATH
        find_d8() {
          local sdk_bt=""
          if [[ -n "$ANDROID_SDK_ROOT" && -d "$ANDROID_SDK_ROOT/build-tools" ]]; then
            sdk_bt="$ANDROID_SDK_ROOT/build-tools"
          elif [[ -n "$ANDROID_HOME" && -d "$ANDROID_HOME/build-tools" ]]; then
            sdk_bt="$ANDROID_HOME/build-tools"
          elif [[ -d "$HOME/Android/Sdk/build-tools" ]]; then
            sdk_bt="$HOME/Android/Sdk/build-tools"
          fi
          if [[ -n "$sdk_bt" ]]; then
            D8_BIN=""
            while IFS= read -r dir; do
              if [[ -x "$dir/d8" ]]; then D8_BIN="$dir/d8"; break; fi
            done < <(ls -1 "$sdk_bt" | sort -Vr | sed "s|^|$sdk_bt/|")
          fi
        }
        D8_BIN=""
        find_d8
        if [[ -z "$D8_BIN" ]]; then
          if command -v d8 >/dev/null 2>&1; then D8_BIN="$(command -v d8)"; fi
        fi
        if [[ -n "$D8_BIN" ]]; then
          echo "Using d8 at $D8_BIN"
          echo "Packaging classes into jar for d8..."
          (cd "$STAGE_DIR/java-classes" && jar cf "$STAGE_DIR/java-classes.jar" .)
          "$D8_BIN" --output "$STAGE_DIR" "$STAGE_DIR/java-classes.jar"
          # Normalize d8 output to $STAGE_DIR/classes.dex
          if [[ -f "$STAGE_DIR/classes.dex" ]]; then
            echo "d8 produced classes.dex at $STAGE_DIR/classes.dex"
          else
            # Some d8 versions output into a subdir named 'classes.dex' under classes/
            if [[ -f "$STAGE_DIR/classes/classes.dex" ]]; then
              mv -f "$STAGE_DIR/classes/classes.dex" "$STAGE_DIR/classes.dex"
              echo "d8 output normalized from classes/classes.dex"
            fi
          fi
          if [[ ! -s "$STAGE_DIR/classes.dex" ]]; then
            echo "ERROR: d8 did not produce a usable classes.dex"
            exit 1
          fi
          DEX_SIZE_BEFORE=$(stat -c%s "$STAGE_DIR/classes.dex" 2>/dev/null || wc -c < "$STAGE_DIR/classes.dex")
          DEX_SHA_BEFORE=$(sha1sum "$STAGE_DIR/classes.dex" 2>/dev/null | awk '{print $1}')
          echo "Verifying classes.dex contains UiHelpers..."
          if ! strings "$STAGE_DIR/classes.dex" | grep -q "org/gltron/game/UiHelpers"; then
            echo "ERROR: UiHelpers not found in classes.dex; aborting to avoid runtime crash"
            exit 1
          fi
          echo "classes.dex BEFORE packaging: size=$DEX_SIZE_BEFORE sha1=$DEX_SHA_BEFORE"
        elif command -v dx >/dev/null 2>&1; then
          echo "Trying dx with jar input..."
          (cd "$STAGE_DIR/java-classes" && jar cf "$STAGE_DIR/java-classes.jar" .)
          if dx --dex --output "$STAGE_DIR/classes.dex" "$STAGE_DIR/java-classes.jar"; then
            echo "dx jar -> classes.dex OK"
          else
            echo "dx jar failed; trying dx with class directory..."
            if dx --dex --output "$STAGE_DIR/classes.dex" "$STAGE_DIR/java-classes/"; then
              echo "dx dir -> classes.dex OK"
            else
              echo "dx dir failed; trying dx with class list..."
              find "$STAGE_DIR/java-classes" -type f -name "*.class" > "$STAGE_DIR/classlist.txt"
              if dx --dex --output "$STAGE_DIR/classes.dex" @"$STAGE_DIR/classlist.txt"; then
                echo "dx list -> classes.dex OK"
              else
                echo "ERROR: dx failed to produce classes.dex"
                exit 1
              fi
            fi
          fi
        else
          echo "ERROR: Neither d8 nor dx (Android SDK) found. Install build-tools and/or set ANDROID_SDK_ROOT."
          exit 1
        fi
      else
        echo "Warning: javac failed; skipping Java helper"
      fi
    fi
  fi
fi

# If an existing prebuilt classes.dex is provided, copy it as fallback
if [[ -f "$ROOT_DIR/tools/minimal-classes.dex" && ! -f "$STAGE_DIR/classes.dex" ]]; then
  cp "$ROOT_DIR/tools/minimal-classes.dex" "$STAGE_DIR/classes.dex" || true
fi

# Validate package name (Java package format): segments of [a-z][a-z0-9_]*, at least 2 segments, no leading/trailing dots
if [[ ! "$PKG" =~ ^[a-z][a-z0-9]*(\.[a-z][a-z0-9]*)+$ ]]; then
  err "Invalid ANDROID_APP_ID/package: '$PKG'. Use lowercase dotted identifiers like 'org.gltron.game'"
fi

# Create AndroidManifest.xml using a temporary file
MANIFEST_TEMP=$(mktemp)
cat > "$MANIFEST_TEMP" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="org.gltron.game"
    android:versionCode="1"
    android:versionName="1.0">

    <uses-sdk android:minSdkVersion="29" android:targetSdkVersion="35" />

    <application
        android:label="GLTron"
        android:hasCode="true"
        android:allowBackup="true"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen">

        <activity
            android:name="android.app.NativeActivity"
            android:label="GLTron"
            android:exported="true"
            android:configChanges="keyboard|keyboardHidden|orientation|screenSize">
            android:launchMode="singleTask" />

            <meta-data android:name="android.app.lib_name" android:value="gltron" />

            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF

# Move the temporary file to the final location
mv "$MANIFEST_TEMP" "$STAGE_DIR/manifest/AndroidManifest.xml" || {
  err "Failed to create AndroidManifest.xml"
}

# Print classes.dex info before APK packaging
if [[ -f "$STAGE_DIR/classes.dex" ]]; then
  DEX_SIZE_BEFORE=${DEX_SIZE_BEFORE:-$(stat -c%s "$STAGE_DIR/classes.dex" 2>/dev/null || wc -c < "$STAGE_DIR/classes.dex")}
  DEX_SHA_BEFORE=${DEX_SHA_BEFORE:-$(sha1sum "$STAGE_DIR/classes.dex" 2>/dev/null | awk '{print $1}')}
  echo "[build_android_apk] Packing classes.dex: size=$DEX_SIZE_BEFORE sha1=$DEX_SHA_BEFORE"
else
  echo "ERROR: classes.dex missing before packaging"
  exit 1
fi

# ------------------------------------------------------------------
# Stage assets into APK under /assets so AAssetManager can load them
# ------------------------------------------------------------------
mkdir -p "$STAGE_DIR/assets"
# Copy root-level files used by the game
cp -a "$ROOT_DIR"/menu.txt "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/settings.txt "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/xenotron.ftx "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/tron.mtl "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/t-u-low.obj "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/gltron.it "$STAGE_DIR/assets/" 2>/dev/null || true
# Copy textures, sounds, shaders matching project expectations
cp -a "$ROOT_DIR"/*.sgi "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/*.ftx "$STAGE_DIR/assets/" 2>/dev/null || true
cp -a "$ROOT_DIR"/*.wav "$STAGE_DIR/assets/" 2>/dev/null || true
# Create strings.xml using a temporary file
STRINGS_TEMP=$(mktemp)
cat > "$STRINGS_TEMP" <<EOF
<resources>
    <string name="gltron">$APP_NAME</string>
</resources>
EOF

# Move the temporary file to the final location
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

# Package resources and manifest
log "Using package: $PKG"
log "Min SDK: $MIN_SDK, Target SDK: $TARGET_SDK, ABI: $ABI, LIB: $LIB_NAME"
log "Manifest: $STAGE_DIR/manifest/AndroidManifest.xml"
# Ensure temp files are cleaned on exit
cleanup() { rm -rf "$STAGE_DIR" "$UNALIGNED_APK" "$ALIGNED_APK" 2>/dev/null || true; }
trap cleanup EXIT
UNALIGNED_APK="$(mktemp "${TMPDIR:-/tmp}/gltron-unaligned.XXXXXX.apk")"
rm -f "$APK_OUT"

if [[ -x "$AAPT" ]]; then
  log "Building APK with aapt (preferred for v1+v2 signing)"
  "$AAPT" package -f \
    -M "$STAGE_DIR/manifest/AndroidManifest.xml" \
    -S "$STAGE_DIR/res" \
    -A "$STAGE_DIR/assets" \
    -I "$ANDROID_SDK_ROOT/platforms/android-$TARGET_SDK/android.jar" \
    -F "$UNALIGNED_APK" || {
    err "Failed to package APK with aapt"
  }
  # Add classes.dex and native libs to the APK in one go
  (cd "$STAGE_DIR" && "$AAPT" add "$UNALIGNED_APK" "classes.dex" "lib/$ABI/$SO_NAME" "lib/$ABI/libc++_shared.so") || {
    err "Failed to add classes.dex and native libs with aapt"
  }
elif [[ -x "$AAPT2" ]]; then
  log "Building APK with aapt2 (fallback)"
  mkdir -p "$STAGE_DIR/compiled-res"
  "$AAPT2" compile -o "$STAGE_DIR/compiled-res" "$STAGE_DIR/res/values/strings.xml" || {
    err "Failed to compile resources with aapt2"
  }
  COMPILED_RES_FILES=("$STAGE_DIR"/compiled-res/*.flat)
  "$AAPT2" link -o "$UNALIGNED_APK" \
    --manifest "$STAGE_DIR/manifest/AndroidManifest.xml" \
    -I "$ANDROID_SDK_ROOT/platforms/android-$TARGET_SDK/android.jar" \
    --min-sdk-version "$MIN_SDK" \
    --target-sdk-version "$TARGET_SDK" \
    "${COMPILED_RES_FILES[@]}" || {
    err "Failed to link resources with aapt2"
  }
  (cd "$STAGE_DIR" && zip -qur "$UNALIGNED_APK" assets lib || {
    err "Failed to add assets and libs to APK"
  })
else
  err "Neither aapt nor aapt2 found in $BUILD_TOOLS_DIR"
fi

# Verify the unaligned APK was created
if [[ ! -f "$UNALIGNED_APK" ]]; then
  err "Failed to create unaligned APK"
fi

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

# Verify classes.dex inside APK matches what we built
TMP_DEX_APK=$(mktemp)
unzip -p "$APK_OUT" classes.dex > "$TMP_DEX_APK" 2>/dev/null || true
if [[ -s "$TMP_DEX_APK" ]]; then
  DEX_SIZE_AFTER=$(stat -c%s "$TMP_DEX_APK" 2>/dev/null || wc -c < "$TMP_DEX_APK")
  DEX_SHA_AFTER=$(sha1sum "$TMP_DEX_APK" 2>/dev/null | awk '{print $1}')
  log "APK classes.dex: size=$DEX_SIZE_AFTER sha1=$DEX_SHA_AFTER"
  if ! strings "$TMP_DEX_APK" | grep -q "org/gltron/game/UiHelpers"; then
    err "APK classes.dex does not contain UiHelpers; build will likely crash at runtime"
  fi
else
  log "WARNING: No classes.dex found inside APK"
fi
rm -f "$TMP_DEX_APK"

log "APK built and signed (v2 verified): $APK_OUT"
echo "$APK_OUT"
