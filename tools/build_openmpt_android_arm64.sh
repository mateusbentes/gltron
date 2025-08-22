#!/usr/bin/env bash
set -euo pipefail

# Logging helpers
log() { echo "[openmpt-android] $*"; }
err() { echo "[openmpt-android][ERROR] $*" >&2; }
run() { if [[ "${VERBOSE:-0}" == 1 ]]; then set -x; fi; "$@"; if [[ "${VERBOSE:-0}" == 1 ]]; then set +x; fi; }

# Build libopenmpt and its dependencies (ogg, vorbis, flac, libsndfile) for Android (default arm64-v8a)
# using the Android NDK toolchain. Static libraries by default.
#
# Assumptions:
# - Sources exist locally under android-dependencies/ in subfolders:
#     ogg, vorbis, flac, libsndfile, openmpt
# - ANDROID_NDK or ANDROID_NDK_HOME is set. If not, default to /home/mateus/Android/Sdk/ndk/29.0.13846066
# - Default API level is 21 (override via ANDROID_API)
# - Outputs staged into android-dependencies/$ANDROID_ABI/prefix/{lib,include}
#
# Usage examples:
#   tools/build_openmpt_android_arm64.sh
#   ANDROID_API=24 tools/build_openmpt_android_arm64.sh
#   CLEAN=1 tools/build_openmpt_android_arm64.sh
#   VERBOSE=1 tools/build_openmpt_android_arm64.sh
#   BUILD_SHARED=1 tools/build_openmpt_android_arm64.sh
#   ANDROID_ABI=armeabi-v7a tools/build_openmpt_android_arm64.sh
#   SKIP_DEPS=1 tools/build_openmpt_android_arm64.sh  # build only openmpt using existing deps

ROOT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
DEPS_DIR="$ROOT_DIR/android-dependencies"
ANDROID_ABI=${ANDROID_ABI:-arm64-v8a}
OUT_DIR="$DEPS_DIR/$ANDROID_ABI"
log "Target ANDROID_ABI=$ANDROID_ABI (default arm64-v8a). ANDROID_API=${ANDROID_API:-29}"
PREFIX="$OUT_DIR/prefix"
LIBDIR="$PREFIX/lib"
INCDIR="$PREFIX/include"
BUILD_SHARED=${BUILD_SHARED:-0}
BUILD_VORBISENC=${BUILD_VORBISENC:-1}
ANDROID_API=${ANDROID_API:-29}
VERBOSE=${VERBOSE:-0}
CLEAN=${CLEAN:-0}
SKIP_DEPS=${SKIP_DEPS:-0}

ENV_EXPORT_FILE="$OUT_DIR/env.sh"

require_dir() {
  if [[ ! -d "$1" ]]; then err "Missing directory: $1"; exit 1; fi
}

maybe_autogen() {
  local srcdir="$1"
  if [[ -x "$srcdir/autogen.sh" ]]; then (cd "$srcdir" && run ./autogen.sh); fi
  if [[ -x "$srcdir/bootstrap" ]]; then (cd "$srcdir" && run ./bootstrap); fi
}

setup_toolchain() {
  export CC_FOR_BUILD="${CC_FOR_BUILD:-gcc}"
  : "${ANDROID_NDK:=${ANDROID_NDK_HOME:-}}"
  if [[ -z "${ANDROID_NDK}" ]]; then
    ANDROID_NDK="/home/mateus/Android/Sdk/ndk/29.0.13846066"
    log "ANDROID_NDK not set; defaulting to $ANDROID_NDK"
  fi
  if [[ ! -d "$ANDROID_NDK" ]]; then
    err "ANDROID_NDK not found at: $ANDROID_NDK"
    exit 1
  fi

  case "$ANDROID_ABI" in
    arm64-v8a) TARGET_HOST=aarch64-linux-android ;;
    armeabi-v7a) TARGET_HOST=armv7a-linux-androideabi ;;
    x86) TARGET_HOST=i686-linux-android ;;
    x86_64) TARGET_HOST=x86_64-linux-android ;;
    *) err "Unsupported ANDROID_ABI: $ANDROID_ABI"; exit 1 ;;
  esac
  log "Using target triple: $TARGET_HOST$ANDROID_API"
  export TARGET_HOST ANDROID_API

  local host_tag
  if [[ -d "$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64" ]]; then host_tag=linux-x86_64
  elif [[ -d "$ANDROID_NDK/toolchains/llvm/prebuilt/darwin-x86_64" ]]; then host_tag=darwin-x86_64
  elif [[ -d "$ANDROID_NDK/toolchains/llvm/prebuilt/darwin-arm64" ]]; then host_tag=darwin-arm64
  elif [[ -d "$ANDROID_NDK/toolchains/llvm/prebuilt/windows-x86_64" ]]; then host_tag=windows-x86_64
  else err "Unable to locate NDK llvm toolchain prebuilt dir under $ANDROID_NDK/toolchains/llvm/prebuilt"; exit 1; fi
  export TOOLCHAIN="$ANDROID_NDK/toolchains/llvm/prebuilt/$host_tag"

  export PATH="$TOOLCHAIN/bin:$PATH"
  export AR="$TOOLCHAIN/bin/llvm-ar"
  export RANLIB="$TOOLCHAIN/bin/llvm-ranlib"
  export STRIP="$TOOLCHAIN/bin/llvm-strip"
  export CC="$TOOLCHAIN/bin/${TARGET_HOST}${ANDROID_API}-clang"
  export CXX="$TOOLCHAIN/bin/${TARGET_HOST}${ANDROID_API}-clang++"
  export LD="$TOOLCHAIN/bin/ld"
  export AS="$TOOLCHAIN/bin/llvm-as"
  export NM="$TOOLCHAIN/bin/llvm-nm"
  export READELF="$TOOLCHAIN/bin/llvm-readelf"
  export SYSROOT="$TOOLCHAIN/sysroot"

  export CPPFLAGS="--sysroot=$SYSROOT"
  export CFLAGS="$CPPFLAGS -fPIC -O2 -fvisibility=hidden"
  export CXXFLAGS="$CFLAGS -fvisibility-inlines-hidden"
  export LDFLAGS="--sysroot=$SYSROOT"

  export PKG_CONFIG="pkg-config"
  export PKG_CONFIG_LIBDIR="$LIBDIR/pkgconfig"
  export PKG_CONFIG_PATH="$PKG_CONFIG_LIBDIR"

  # Autoconf cache to avoid running test executables
  export ac_cv_func_malloc_0_nonnull=yes
  export ac_cv_func_realloc_0_nonnull=yes
  export ac_cv_c_bigendian=no
  export lt_cv_deplibs_check_method=pass_all

  mkdir -p "$LIBDIR/pkgconfig" "$INCDIR" "$OUT_DIR/build"
}

common_autotools_flags() {
  local shared_flag
  if [[ "$BUILD_SHARED" == 1 ]]; then
    shared_flag="--enable-shared --disable-static"
  else
    shared_flag="--disable-shared --enable-static"
  fi
  echo "--host=$TARGET_HOST --prefix=$PREFIX $shared_flag"
}

build_autotools() {
  local srcdir="$1"; shift
  local extraconf=("$@")
  local blddir="$OUT_DIR/build/$(basename "$srcdir")"

  # Aggressive cleanup of previous configuration
  if [[ -d "$blddir" ]]; then
    if [[ -f "$blddir/Makefile" || -f "$blddir/config.status" || -f "$blddir/config.log" || -f "$blddir/libtool" ]]; then
      (cd "$blddir" && make distclean) || rm -rf "$blddir"
    fi
  fi
  # Also ensure the source tree is clean if it was configured in-tree earlier
  if [[ -f "$srcdir/Makefile" || -f "$srcdir/config.status" || -f "$srcdir/config.log" || -f "$srcdir/libtool" ]]; then
    (cd "$srcdir" && make distclean) || true
    rm -f "$srcdir/Makefile" "$srcdir/config.status" "$srcdir/config.log" "$srcdir/config.cache" "$srcdir/libtool" || true
    rm -rf "$srcdir/autom4te.cache" || true
  fi

  mkdir -p "$blddir"
  pushd "$blddir" >/dev/null
  maybe_autogen "$srcdir"
  local build_triplet
  build_triplet=$(gcc -dumpmachine 2>/dev/null || echo unknown)

  # Create a config.site to force cross-compile cache answers and avoid running test binaries
  cat >"$blddir/config.site" <<EOF
ac_cv_prog_cc_works=yes
ac_cv_prog_cc_cross=yes
ac_cv_exeext=
ac_cv_file__dev_zero=yes
ac_cv_c_bigendian=no
ac_cv_header_sys_types_h=yes
ac_cv_func_malloc_0_nonnull=yes
ac_cv_func_realloc_0_nonnull=yes
ac_cv_func_memset=yes
ac_cv_func_memcpy=yes
lt_cv_deplibs_check_method=pass_all
EOF
  export CONFIG_SITE="$blddir/config.site"

  run "$srcdir/configure" $(common_autotools_flags) --build="$build_triplet" \
    "CC=$CC" "CXX=$CXX" "AR=$AR" "RANLIB=$RANLIB" "STRIP=$STRIP" \
    "CC_FOR_BUILD=$CC_FOR_BUILD" \
    CPPFLAGS="$CPPFLAGS" CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS" \
    "${extraconf[@]}"
  run make -j"$(nproc)"
  run make install
  popd >/dev/null
}

build_cmake() {
  local srcdir="$1"; shift
  local extracmake=("$@")
  local blddir="$OUT_DIR/build/$(basename "$srcdir")-cmake"
  # If previous CMake cache exists, nuke the build directory
  if [[ -d "$blddir" && -f "$blddir/CMakeCache.txt" ]]; then rm -rf "$blddir"; fi
  mkdir -p "$blddir"
  pushd "$blddir" >/dev/null
  local BUILD_SHARED_LIBS
  if [[ "$BUILD_SHARED" == 1 ]]; then BUILD_SHARED_LIBS=ON; else BUILD_SHARED_LIBS=OFF; fi
  run cmake -G "Ninja" \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ANDROID_ABI" \
    -DANDROID_PLATFORM=android-$ANDROID_API \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DBUILD_SHARED_LIBS=$BUILD_SHARED_LIBS \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CXXFLAGS" -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS" -DCMAKE_SHARED_LINKER_FLAGS="$LDFLAGS" \
    -DCMAKE_FIND_ROOT_PATH="$PREFIX;$SYSROOT" \
    -DCMAKE_PREFIX_PATH="$PREFIX" \
    "${extracmake[@]}" \
    "$srcdir"
  run cmake --build . --parallel
  run cmake --install .
  popd >/dev/null
}

build_ogg() {
  local src="$DEPS_DIR/ogg"
  require_dir "$src"
  log "Building libogg"
  if [[ -f "$src/CMakeLists.txt" ]]; then
    build_cmake "$src" \
      -DINSTALL_DOCS=OFF -DINSTALL_PKG_CONFIG_MODULE=OFF -DINSTALL_MANPAGES=OFF
  else
    build_autotools "$src" --disable-shared --enable-static --disable-examples --disable-tests
  fi
}

build_vorbis() {
  local src="$DEPS_DIR/vorbis"
  require_dir "$src"
  log "Building libvorbis"
  if [[ -f "$src/CMakeLists.txt" ]]; then
    build_cmake "$src" \
      -DOGG_INCLUDE_DIR="$INCDIR" -DOGG_LIBRARY="$LIBDIR/libogg.a" \
      -DINSTALL_DOCS=OFF -DINSTALL_MANPAGES=OFF -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF
  else
    local extra=(--with-ogg="$PREFIX" --disable-oggtest --disable-docs --disable-examples --disable-tests)
    if [[ "$BUILD_VORBISENC" == 0 ]]; then extra+=(--disable-encoders); fi
    build_autotools "$src" "${extra[@]}"
  fi
}

build_flac() {
  local src="$DEPS_DIR/flac"
  require_dir "$src"
  log "Building FLAC"
  if [[ -f "$src/CMakeLists.txt" ]]; then
    build_cmake "$src" \
      -DBUILD_CXXLIBS=OFF -DBUILD_DOCS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DBUILD_PROGRAMS=OFF -DINSTALL_MANPAGES=OFF \
      -DWITH_OGG=ON -DOGG_INCLUDE_DIR="$INCDIR" -DOGG_LIBRARY="$LIBDIR/libogg.a"
  else
    build_autotools "$src" --disable-oggtest --with-ogg="$PREFIX" \
      --disable-cpplibs --disable-doxygen-docs --disable-xmms-plugin --disable-sse --disable-altivec --disable-asm-optimizations \
      --disable-examples --disable-tests
  fi
}

build_libsndfile() {
  local src="$DEPS_DIR/libsndfile"
  require_dir "$src"
  log "Building libsndfile"
  if [[ -f "$src/CMakeLists.txt" ]]; then
    build_cmake "$src" \
      -DBUILD_PROGRAMS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF -DINSTALL_MANPAGES=OFF \
      -DENABLE_EXTERNAL_LIBS=ON -DENABLE_EXPERIMENTAL=OFF -DENABLE_MPEG=OFF -DENABLE_OPUS=ON \
      -DENABLE_OGG=ON -DENABLE_VORBIS=ON -DENABLE_FLAC=ON \
      -DOPUS_INCLUDE_DIR="$INCDIR" -DOPUS_LIBRARY="$LIBDIR/libopus.a" -DOpus_INCLUDE_DIR="$INCDIR" -DOpus_LIBRARY="$LIBDIR/libopus.a"
  else
    build_autotools "$src" --disable-sqlite --disable-full-suite --disable-external-libs=no --with-ogg="$PREFIX" --with-vorbis="$PREFIX" --with-flac="$PREFIX" --with-opus="$PREFIX"
  fi
}

build_openmpt() {
  local src="$DEPS_DIR/openmpt"
  require_dir "$src"
  log "Building libopenmpt (make-based)"

  # Candidate directories containing Makefile
  local candidates=("$src" "$src/libopenmpt" "$src/build/make")
  local mdir=""
  for d in "${candidates[@]}"; do
    if [[ -f "$d/Makefile" ]]; then mdir="$d"; break; fi
  done
  if [[ -z "$mdir" ]]; then
    err "Could not find a Makefile for openmpt under: $src (looked in: ${candidates[*]})"
    err "Please ensure your openmpt source contains a make-based build."
    exit 1
  fi

  pushd "$mdir" >/dev/null
  # Clean previous build if any
  make distclean >/dev/null 2>&1 || make clean >/dev/null 2>&1 || true

  # Compose flags and variables
  local INC_FLAGS="-I$INCDIR"
  local LIB_FLAGS="-L$LIBDIR"
  local EXTRA_LIBS="-lsndfile -lvorbis -lvorbisfile -lvorbisenc -logg -lFLAC -lopus"

  # Many makefiles honor these environment variables
  run env \
    CC="$CC" CXX="$CXX" AR="$AR" RANLIB="$RANLIB" STRIP="$STRIP" \
    CPPFLAGS="$CPPFLAGS $INC_FLAGS" CFLAGS="$CFLAGS $INC_FLAGS" CXXFLAGS="$CXXFLAGS $INC_FLAGS" \
    LDFLAGS="$LDFLAGS $LIB_FLAGS" LIBS="$EXTRA_LIBS" \
    make -j"$(nproc)"

  # Locate resulting static library
  local built_lib=""
  if [[ -f "$mdir/libopenmpt.a" ]]; then built_lib="$mdir/libopenmpt.a"; fi
  if [[ -z "$built_lib" && -f "$mdir/bin/libopenmpt.a" ]]; then built_lib="$mdir/bin/libopenmpt.a"; fi
  if [[ -z "$built_lib" ]]; then
    # Try find under dir
    built_lib=$(find . -maxdepth 3 -name 'libopenmpt.a' | head -n1 || true)
  fi
  if [[ -z "$built_lib" ]]; then
    err "libopenmpt.a was not produced by make in $mdir"
    exit 1
  fi

  mkdir -p "$LIBDIR" "$INCDIR/libopenmpt"
  run cp -f "$built_lib" "$LIBDIR/"

  # Try to install/copy public headers into include/libopenmpt
  # Common header locations: include/, libopenmpt/ (public headers like libopenmpt.h)
  local hdr_src=""
  if [[ -d "$src/include" ]]; then hdr_src="$src/include"; fi
  if [[ -z "$hdr_src" && -d "$src/libopenmpt" ]]; then hdr_src="$src/libopenmpt"; fi
  if [[ -n "$hdr_src" ]]; then
    # Copy only libopenmpt* headers
    run rsync -a --include 'libopenmpt*.h' --include 'libopenmpt/*.h' --exclude '*' "$hdr_src/" "$INCDIR/libopenmpt/" || true
  fi
  # Fallback: scan tree for libopenmpt*.h
  mapfile -t found_hdrs < <(find "$src" -type f -name 'libopenmpt*.h' 2>/dev/null || true)
  if [[ ${#found_hdrs[@]} -gt 0 ]]; then
    for h in "${found_hdrs[@]}"; do
      run cp -f "$h" "$INCDIR/libopenmpt/"
    done
  fi
  if [[ ! -f "$INCDIR/libopenmpt/libopenmpt.h" ]]; then
    log "Warning: libopenmpt.h not found under $src; headers may be incomplete."
  fi
  popd >/dev/null
}

write_pkgconfig_files() {
  log "Writing pkg-config stubs (for convenience)"
  mkdir -p "$LIBDIR/pkgconfig"
  cat >"$LIBDIR/pkgconfig/openmpt.pc" <<EOF
prefix=$PREFIX
exec_prefix=
libdir=
includedir=$INCDIR

Name: libopenmpt
Description: module music library
Version: local
Libs: -L$LIBDIR -lopenmpt -lsndfile -lvorbis -lvorbisfile -lvorbisenc -logg -lFLAC
Cflags: -I$INCDIR
EOF
}

write_env_exports() {
  log "Writing environment exports to $ENV_EXPORT_FILE"
  cat >"$ENV_EXPORT_FILE" <<EOF
# Source this file to help CMake find the locally built libraries
export CMAKE_PREFIX_PATH="$PREFIX:${CMAKE_PREFIX_PATH:-}"
export OpenMPT_LIBRARY="$LIBDIR/libopenmpt.a"
export OpenMPT_INCLUDE_DIR="$INCDIR"
EOF
}

validate_outputs() {
  log "Validating output archives"
  local libs=("libogg.a" "libvorbis.a" "libvorbisfile.a" "libFLAC.a" "libsndfile.a" "libopenmpt.a")
  for lib in "${libs[@]}"; do
    local p="$LIBDIR/$lib"
    if [[ ! -f "$p" ]]; then err "Missing expected library: $p"; exit 1; fi
    if ! run "$AR" t "$p" >/dev/null 2>&1; then
      err "Archive seems invalid: $p"; exit 1
    fi
  done
  log "All expected libraries are present and valid archives."
}

prepare() {
  require_dir "$DEPS_DIR"
  require_dir "$DEPS_DIR/ogg"
  require_dir "$DEPS_DIR/vorbis"
  require_dir "$DEPS_DIR/flac"
  require_dir "$DEPS_DIR/libsndfile"
  require_dir "$DEPS_DIR/opus"
  require_dir "$DEPS_DIR/openmpt"
  if [[ "$CLEAN" == 1 ]]; then
    log "Cleaning previous builds in $OUT_DIR"
    rm -rf "$OUT_DIR"
  fi
  mkdir -p "$OUT_DIR" "$PREFIX" "$LIBDIR" "$INCDIR"
}

build_opus() {
  local src="$DEPS_DIR/opus"
  require_dir "$src"
  log "Building Opus"
  if [[ -f "$src/CMakeLists.txt" ]]; then
    build_cmake "$src" \
      -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DOPUS_BUILD_TESTING=OFF
  else
    build_autotools "$src" --disable-shared --enable-static --disable-doc --disable-examples --disable-tests
  fi
}

main() {
  prepare
  setup_toolchain
  if [[ "$SKIP_DEPS" == 0 ]]; then
    build_ogg
    build_vorbis
    build_flac
    build_opus
    build_libsndfile
  else
    log "Skipping dependencies build as requested (SKIP_DEPS=1)"
  fi
  build_openmpt
  validate_outputs
  write_pkgconfig_files || true
  write_env_exports || true
  log "Done. Staged outputs under: $PREFIX"
  log "Libs: $LIBDIR"
  log "Includes: $INCDIR"
  log "Hint: source $ENV_EXPORT_FILE to help CMake find the libraries"
}

main "$@"
