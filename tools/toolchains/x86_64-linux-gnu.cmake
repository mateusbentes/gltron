# Sample CMake toolchain file for cross-compiling Linux desktop x86_64 on an ARM64 host.
# Prerequisites (Debian/Ubuntu):
#   sudo apt-get install gcc-x86-64-linux-gnu g++-x86-64-linux-gnu \
#                        libgl1-mesa-dev:amd64 freeglut3-dev:amd64 libglu1-mesa-dev:amd64 libmikmod-dev:amd64

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# Compilers (use GNU drivers to avoid invoking clang/ndk lld by accident)
set(CMAKE_C_COMPILER   x86_64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER x86_64-linux-gnu-g++)

# Explicitly set binutils and linker to x86_64 variants
set(CMAKE_LINKER  x86_64-linux-gnu-ld)
set(CMAKE_AR      x86_64-linux-gnu-ar)
set(CMAKE_NM      x86_64-linux-gnu-nm)
set(CMAKE_RANLIB  x86_64-linux-gnu-ranlib)
set(CMAKE_STRIP   x86_64-linux-gnu-strip)

# Force link to use gcc/g++ driver
set(CMAKE_C_LINK_EXECUTABLE   "<CMAKE_C_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Optional: help pkg-config locate amd64 .pc files
# If your distro stores multiarch pkg-config in /usr/lib/x86_64-linux-gnu/pkgconfig, enable this:
set(ENV{PKG_CONFIG_DIR} "")
set(ENV{PKG_CONFIG_LIBDIR} "/usr/lib/x86_64-linux-gnu/pkgconfig:/usr/share/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "")

# Prefer find_package to use the cross sysroot; avoid host paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
