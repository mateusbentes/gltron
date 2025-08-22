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
ANDROID_ABI_FORCED="arm64-v8a"
if [[ -n "${ANDROID_ABI:-}" && "${ANDROID_ABI}" != "${ANDROID_ABI_FORCED}" ]]; then
  err "This script only builds for arm64-v8a. You set ANDROID_ABI='${ANDROID_ABI}'."
  exit 1
fi
ANDROID_ABI="${ANDROID_ABI_FORCED}"
OUT_DIR="$DEPS_DIR/$ANDROID_ABI"
log "Enforcing ANDROID_ABI=$ANDROID_ABI (arm64-v8a only). ANDROID_API=${ANDROID_API:-29}"
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
  local p="$1"
  if [[ -z "${p}" ]]; then
    err "require_dir called with empty path (variable unset or empty)"
    err "Callsite hint: ensure DEPS_DIR and expected subpaths are set before invoking build steps."
    exit 1
  fi
  if [[ ! -d "${p}" ]]; then
    err "Missing directory: ${p}"
    exit 1
  fi
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
  *) err "Unsupported ANDROID_ABI: $ANDROID_ABI (this script only supports arm64-v8a)"; exit 1 ;;
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

  # Only set architecture flags that are valid for ARM64
  export CPPFLAGS="--sysroot=$SYSROOT -march=armv8-a"
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
      -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_INCLUDEDIR=include \
      -DINSTALL_PKG_CONFIG_MODULE=ON -DINSTALL_PKGCONFIG_MODULE=ON \
      -DINSTALL_DOCS=OFF -DINSTALL_MANPAGES=OFF
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
      -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_INCLUDEDIR=include \
      -DINSTALL_PKG_CONFIG_MODULE=ON -DINSTALL_PKGCONFIG_MODULE=ON \
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
      -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_INCLUDEDIR=include \
      -DINSTALL_PKG_CONFIG_MODULE=ON -DINSTALL_PKGCONFIG_MODULE=ON \
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
  log "build_openmpt: DEPS_DIR=$DEPS_DIR src=$src"
  require_dir "$src"
  log "Building libopenmpt (CMake-based, enforced Android arm64)"

  # Prefer CMake build for robust cross-compilation
  if [[ -f "$src/CMakeLists.txt" ]]; then
    local cmake_args=(
      -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake"
      -DANDROID_ABI="$ANDROID_ABI"
      -DANDROID_PLATFORM=android-$ANDROID_API
      -DCMAKE_C_FLAGS="-march=armv8-a -mfpu=vfpv3-d16 -mfloat-abi=softfp"
      -DCMAKE_CXX_FLAGS="-march=armv8-a -mfpu=vfpv3-d16 -mfloat-abi=softfp"
      -DOPENMPT_BUILD_SHARED=OFF
      -DOPENMPT_BUILD_STATIC=ON
      -DOPENMPT_BUILD_TESTS=OFF
      -DOPENMPT_BUILD_EXAMPLES=OFF
      -DOPENMPT_BUILD_APPS=OFF
      -DOPENMPT_INSTALL=ON
      -DOPENMPT_USE_PORTAUDIO=OFF
      -DOPENMPT_USE_PORTAUDIO_DYNAMIC=OFF
      -DOPENMPT_USE_SDL2=OFF
      -DOPENMPT_USE_SDL2_DYNAMIC=OFF
      -DOPENMPT_USE_PULSEAUDIO=OFF
      -DOPENMPT_USE_PULSEAUDIO_DYNAMIC=OFF
      -DOPENMPT_USE_ALSA=OFF
      -DOPENMPT_USE_ALSA_DYNAMIC=OFF
      -DOPENMPT_USE_JACK=OFF
      -DOPENMPT_USE_JACK_DYNAMIC=OFF
      -DOPENMPT_USE_MPG123=OFF
      -DOPENMPT_USE_MPG123_DYNAMIC=OFF
      -DOPENMPT_USE_VORBIS=ON
      -DOPENMPT_USE_OPUS=ON
      -DOPENMPT_USE_FLAC=ON
      -DOPENMPT_USE_OGG=ON
      -DOGG_INCLUDE_DIR="$INCDIR"
      -DOGG_LIBRARY="$LIBDIR/libogg.a"
      -DVORBIS_INCLUDE_DIR="$INCDIR"
      -DVORBIS_LIBRARY="$LIBDIR/libvorbis.a"
      -DVORBISFILE_LIBRARY="$LIBDIR/libvorbisfile.a"
      -DVORBISENC_LIBRARY="$LIBDIR/libvorbisenc.a"
      -DOPUS_INCLUDE_DIR="$INCDIR"
      -DOPUS_LIBRARY="$LIBDIR/libopus.a"
      -DFLAC_INCLUDE_DIR="$INCDIR"
      -DFLAC_LIBRARY="$LIBDIR/libFLAC.a"
      -DLIBSNDFILE_INCLUDE_DIR="$INCDIR"
      -DLIBSNDFILE_LIBRARY="$LIBDIR/libsndfile.a"
    )
    build_cmake "$src" "${cmake_args[@]}"
  else
    # Fallback: use upstream GNU Makefile with staged deps and static-only
    log "CMakeLists.txt not found at $src; building with upstream Makefile (static-only)"
    pushd "$src" >/dev/null

    # Prefer our staged deps only
    export PKG_CONFIG=${PKG_CONFIG:-pkg-config}
    export PKG_CONFIG_LIBDIR="$LIBDIR/pkgconfig"
    export PKG_CONFIG_PATH="$PKG_CONFIG_LIBDIR"

    # Force include/lib paths and Android-safe defines
    export CPPFLAGS="-I$INCDIR ${CPPFLAGS:-} -DMPT_OS_ANDROID=1 -DMPT_OS_LINUX=1 -DMPT_WITH_MFC=0 -DMPT_EXCEPTIONS_USE_WINDOWS_SEH=0 -DLIBOPENMPT_BUILD=1"
    export CFLAGS="$CFLAGS -I$INCDIR"
    export CXXFLAGS="$CXXFLAGS -I$INCDIR"
    export LDFLAGS="$LDFLAGS -L$LIBDIR"

    log "make openmpt: PKG_CONFIG_PATH=$PKG_CONFIG_PATH CPPFLAGS=$CPPFLAGS LDFLAGS=$LDFLAGS"

    # Resolve flags from pkg-config to inject directly
    pkgc() { PKG_CONFIG_PATH="$PKG_CONFIG_PATH" PKG_CONFIG_LIBDIR="$PKG_CONFIG_LIBDIR" "$PKG_CONFIG" "$@"; }
    OGG_CFLAGS=$(pkgc --cflags ogg 2>/dev/null || echo "-I$INCDIR"); export OGG_CFLAGS
    OGG_LDFLAGS=$(pkgc --libs-only-L ogg 2>/dev/null || echo "-L$LIBDIR"); export OGG_LDFLAGS
    OGG_LDLIBS=$(pkgc --libs-only-l ogg 2>/dev/null || echo "-logg"); export OGG_LDLIBS
    VORBIS_CFLAGS=$(pkgc --cflags vorbis 2>/dev/null || echo "-I$INCDIR"); export VORBIS_CFLAGS
    VORBIS_LDFLAGS=$(pkgc --libs-only-L vorbis 2>/dev/null || echo "-L$LIBDIR"); export VORBIS_LDFLAGS
    VORBIS_LDLIBS=$(pkgc --libs-only-l vorbis 2>/dev/null || echo "-lvorbis"); export VORBIS_LDLIBS
    VORBISFILE_CFLAGS=$(pkgc --cflags vorbisfile 2>/dev/null || echo "-I$INCDIR"); export VORBISFILE_CFLAGS
    VORBISFILE_LDFLAGS=$(pkgc --libs-only-L vorbisfile 2>/dev/null || echo "-L$LIBDIR"); export VORBISFILE_LDFLAGS
    VORBISFILE_LDLIBS=$(pkgc --libs-only-l vorbisfile 2>/dev/null || echo "-lvorbisfile"); export VORBISFILE_LDLIBS
    FLAC_CFLAGS=$(pkgc --cflags flac 2>/dev/null || echo "-I$INCDIR"); export FLAC_CFLAGS
    FLAC_LDFLAGS=$(pkgc --libs-only-L flac 2>/dev/null || echo "-L$LIBDIR"); export FLAC_LDFLAGS
    FLAC_LDLIBS=$(pkgc --libs-only-l flac 2>/dev/null || echo "-lFLAC"); export FLAC_LDLIBS
    SNDFILE_CFLAGS=$(pkgc --cflags sndfile 2>/dev/null || echo "-I$INCDIR"); export SNDFILE_CFLAGS
    SNDFILE_LDFLAGS=$(pkgc --libs-only-L sndfile 2>/dev/null || echo "-L$LIBDIR"); export SNDFILE_LDFLAGS
    SNDFILE_LDLIBS=$(pkgc --libs-only-l sndfile 2>/dev/null || echo "-lsndfile"); export SNDFILE_LDLIBS

    # Build static library only, disable apps/examples/tests and system codecs we do not want
    run make -j"$(nproc)" \
      STATIC_LIB=1 SHARED_LIB=0 EXAMPLES=0 OPENMPT123=0 TEST=0 \
      NO_ZLIB=1 NO_MPG123=1 \
      NO_OGG=0 NO_VORBIS=0 NO_VORBISFILE=0 NO_FLAC=0 NO_SNDFILE=0 \
      TOOLCHAIN_SUFFIX= \
      CPPFLAGS_OGG="$OGG_CFLAGS" LDFLAGS_OGG="$OGG_LDFLAGS" LDLIBS_OGG="$OGG_LDLIBS" \
      CPPFLAGS_VORBIS="$VORBIS_CFLAGS" LDFLAGS_VORBIS="$VORBIS_LDFLAGS" LDLIBS_VORBIS="$VORBIS_LDLIBS" \
      CPPFLAGS_VORBISFILE="$VORBISFILE_CFLAGS" LDFLAGS_VORBISFILE="$VORBISFILE_LDFLAGS" LDLIBS_VORBISFILE="$VORBISFILE_LDLIBS" \
      CPPFLAGS_FLAC="$FLAC_CFLAGS" LDFLAGS_FLAC="$FLAC_LDFLAGS" LDLIBS_FLAC="$FLAC_LDLIBS" \
      CPPFLAGS_SNDFILE="$SNDFILE_CFLAGS" LDFLAGS_SNDFILE="$SNDFILE_LDFLAGS" LDLIBS_SNDFILE="$SNDFILE_LDLIBS" \
      PREFIX="$PREFIX" || true

    # Locate resulting static lib
    local libCandidate
    libCandidate=$(find bin -maxdepth 2 -name 'libopenmpt.a' -print -quit || true)
    if [[ -z "$libCandidate" ]]; then
      err "libopenmpt.a not found after upstream Makefile build. Check make output above."
      popd >/dev/null
      exit 1
    fi
    mkdir -p "$LIBDIR" "$INCDIR/libopenmpt"
    run cp -f "$libCandidate" "$LIBDIR/libopenmpt.a"
    # Install headers
    if [[ -d "libopenmpt" ]]; then
      find libopenmpt -maxdepth 1 -type f -name 'libopenmpt*.h' -exec cp -f {} "$INCDIR/libopenmpt/" \; || true
    fi

    popd >/dev/null
  fi
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
  log "Preflight check: ROOT_DIR=$ROOT_DIR"
  log "Preflight check: DEPS_DIR=$DEPS_DIR"
  log "Preflight check: ANDROID_ABI=$ANDROID_ABI ANDROID_API=$ANDROID_API"
  log "Preflight check: OUT_DIR=$OUT_DIR"
  log "Preflight check: PREFIX=$PREFIX"
  log "Preflight check: LIBDIR=$LIBDIR"
  log "Preflight check: INCDIR=$INCDIR"

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

self_check() {
  log "Diagnostics mode: printing critical variables and paths"
  echo "ROOT_DIR=$ROOT_DIR"
  echo "DEPS_DIR=$DEPS_DIR"
  echo "OUT_DIR=$OUT_DIR"
  echo "PREFIX=$PREFIX"
  echo "LIBDIR=$LIBDIR"
  echo "INCDIR=$INCDIR"
  echo "ANDROID_ABI=$ANDROID_ABI"
  echo "ANDROID_API=$ANDROID_API"
  echo "ANDROID_NDK=${ANDROID_NDK_HOME:-${ANDROID_NDK:-}}"
  echo "TOOLCHAIN=$TOOLCHAIN"
  echo "SYSROOT=$SYSROOT"
  echo "CC=$CC"
  echo "CXX=$CXX"
  echo "PKG_CONFIG_PATH=$PKG_CONFIG_PATH"
}

main() {
  prepare
  setup_toolchain
  if [[ "${RUN_DIAGNOSTICS:-0}" == 1 ]]; then
    self_check
    log "Diagnostics complete; exiting early as requested (RUN_DIAGNOSTICS=1)"
    exit 0
  fi
  if [[ "$SKIP_DEPS" == 0 ]]; then
    build_ogg
    build_vorbis
    build_flac
    build_opus
    build_libsndfile
  else
    log "Skipping dependencies build as requested (SKIP_DEPS=1)"
  fi

  # Pre-check pkg-config availability for OpenMPT deps
  log "Pre-checking pkg-config .pc files in $LIBDIR/pkgconfig"
  local missing_pc=0
  for pc in ogg.pc vorbis.pc vorbisfile.pc flac.pc sndfile.pc; do
    if [[ ! -f "$LIBDIR/pkgconfig/$pc" ]]; then
      err "Missing pkg-config file: $LIBDIR/pkgconfig/$pc"
      missing_pc=1
    fi
  done
  if [[ $missing_pc -eq 1 ]]; then
    err "One or more .pc files are missing; OpenMPT Makefile will not find deps."
    err "Contents of $LIBDIR/pkgconfig:"; ls -l "$LIBDIR/pkgconfig" || true
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
