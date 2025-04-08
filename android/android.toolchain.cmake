# Android toolchain file for CMake
# This is a simplified version - in practice, you might want to use the one provided by the Android NDK

set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_VERSION 21) # API level
set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a) # Default ABI
set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK})
set(CMAKE_ANDROID_STL_TYPE c++_static)

# Set the compilers
set(CMAKE_C_COMPILER "${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/${HOST_TAG}/bin/clang")
set(CMAKE_CXX_COMPILER "${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/${HOST_TAG}/bin/clang++")

# Set the find root path
set(CMAKE_FIND_ROOT_PATH "${CMAKE_ANDROID_NDK}/toolchains/llvm/prebuilt/${HOST_TAG}/sysroot")

# Only search for libraries and includes in the NDK
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Android specific configurations
add_definitions(-DANDROID)