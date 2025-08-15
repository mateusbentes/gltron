# Modular Main CMakeLists.txt for GLTron
cmake_minimum_required(VERSION 3.10)
project(gltron VERSION 0.7.1 LANGUAGES C CXX)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add cmake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Set CMake policies
if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

# Set OpenGL policy to avoid warnings
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

# Canonical Android detection (avoid false positives)
# Rely only on CMake-provided indicators from the Android toolchain.
set(BUILD_ANDROID OFF)
if(ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(BUILD_ANDROID ON)
  message(STATUS "Android detected via canonical CMake variables (ANDROID/CMAKE_SYSTEM_NAME)")
else()
  message(STATUS "Building for non-Android platform")
endif()

# Platform-specific configuration
if(BUILD_ANDROID)
  # Do not force old-style NDK paths; modern toolchains provide includes/libs.
  # Check if Android headers are available
  include(CheckIncludeFile)
  check_include_file(android/log.h ANDROID_LOG_H_AVAILABLE)
  check_include_file(jni.h ANDROID_JNI_H_AVAILABLE)
  if(ANDROID_LOG_H_AVAILABLE)
    message(STATUS "Android log.h available")
    add_definitions(-DANDROID_LOG_H_AVAILABLE)
  else()
    message(WARNING "Android log.h not found, using printf for logging")
  endif()
  if(ANDROID_JNI_H_AVAILABLE)
    message(STATUS "Android jni.h available")
    add_definitions(-DHAVE_JNI_H)
    add_compile_definitions(HAVE_JNI_H)
  else()
    message(WARNING "Android jni.h not found; JNI support disabled for this build")
  endif()
  # Android platform setup
  include_directories(${CMAKE_SOURCE_DIR}/android/include)
  add_definitions(-DANDROID -D__ANDROID__)
  add_compile_definitions(ANDROID __ANDROID__)
  add_definitions(-DGLTRON_BUILD_ANDROID=1)
  message(STATUS "GLTRON_BUILD_ANDROID is set to: 1")
  # Set Android-specific variables if not already set
  if(NOT ANDROID_ABI AND CMAKE_ANDROID_ARCH_ABI)
    set(ANDROID_ABI ${CMAKE_ANDROID_ARCH_ABI})
  endif()
  if(NOT ANDROID_PLATFORM AND CMAKE_ANDROID_API)
    set(ANDROID_PLATFORM "android-${CMAKE_ANDROID_API}")
  endif()
  # Default values for Android
  if(NOT ANDROID_ABI)
    set(ANDROID_ABI "arm64-v8a")
    message(STATUS "Using default Android ABI: ${ANDROID_ABI}")
  endif()
  if(NOT ANDROID_PLATFORM)
    set(ANDROID_PLATFORM "android-24")
    message(STATUS "Using default Android platform: ${ANDROID_PLATFORM}")
  endif()
  message(STATUS "=== Android Build Configuration ===")
  message(STATUS "Android ABI: ${ANDROID_ABI}")
  message(STATUS "Android Platform: ${ANDROID_PLATFORM}")
  message(STATUS "Android NDK: ${ANDROID_NDK}")
  message(STATUS "Toolchain File: ${CMAKE_TOOLCHAIN_FILE}")
  message(STATUS "===================================")
else()
  add_definitions(-DGLTRON_BUILD_ANDROID=0)
  message(STATUS "GLTRON_BUILD_ANDROID is set to: 0")
  if(APPLE)
    set(MACOSX TRUE)
    message(STATUS "Building for macOS platform")
  elseif(WIN32)
    message(STATUS "Building for Windows platform")
  elseif(UNIX)
    set(LINUX TRUE)
    message(STATUS "Building for Linux/Desktop platform")
  else()
    message(FATAL_ERROR "Unsupported platform")
  endif()
endif()

# Build options
option(ENABLE_STATIC_LINKING "Enable static linking" OFF)
option(LOCAL_DATA "Use local data directory" ON)
option(ENABLE_SOUND "Enable sound support" ON)
option(ENABLE_TTF "Enable SDL2_ttf support" ON)
option(ENABLE_XMP "Enable libxmp support for IT file playback" ON)

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Force LOCAL_DATA to be ON during development
set(LOCAL_DATA ON CACHE BOOL "Use local data directory" FORCE)
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "LOCAL_DATA is set to: ${LOCAL_DATA}")
message(STATUS "ENABLE_SOUND is set to: ${ENABLE_SOUND}")
message(STATUS "BUILD_ANDROID is set to: ${BUILD_ANDROID}")
message(STATUS "ENABLE_TTF is set to: ${ENABLE_TTF}")
message(STATUS "ENABLE_XMP is set to: ${ENABLE_XMP}")

# Add compiler definitions
if(BUILD_ANDROID)
  add_definitions(-DANDROID -D__ANDROID__ -DIS_ANDROID=1)
  add_compile_definitions(ANDROID __ANDROID__ IS_ANDROID=1)
endif()

if(LOCAL_DATA)
  add_definitions(-DLOCAL_DATA)
  add_compile_definitions(LOCAL_DATA)
  message(STATUS "LOCAL_DATA definition added")
endif()

# Platform-specific package finding
if(BUILD_ANDROID)
  # Android-specific setup
  if(NOT ANDROID_NDK AND NOT CMAKE_ANDROID_NDK)
    message(WARNING "Android NDK path not explicitly set. Relying on toolchain configuration.")
  endif()

  # Set minimum API level
  if(NOT ANDROID_MIN_API)
    set(ANDROID_MIN_API 24 CACHE STRING "Minimum Android API level")
  endif()

  # Set default ABI if not specified
  if(NOT CMAKE_ANDROID_ARCH_ABI AND NOT ANDROID_ABI)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
    set(ANDROID_ABI arm64-v8a)
  endif()

  # Find Android native libraries with better error handling
  find_library(ANDROID_LIB android)
  find_library(LOG_LIB log)
  find_library(EGL_LIB EGL)
  find_library(GLES_LIB GLESv2)

  # Verify critical Android libraries
  if(NOT ANDROID_LIB)
    message(FATAL_ERROR "Android native library not found. Check NDK configuration.")
  endif()

  if(NOT LOG_LIB)
    message(FATAL_ERROR "Android log library not found. Check NDK configuration.")
  endif()

  if(NOT EGL_LIB)
    message(WARNING "EGL library not found. OpenGL ES may not work properly.")
  endif()

  if(NOT GLES_LIB)
    message(WARNING "GLESv2 library not found. OpenGL ES may not work properly.")
  endif()

  # Android audio support
  if(ENABLE_SOUND)
    find_library(OPENSLES_LIB OpenSLES)
    if(OPENSLES_LIB)
      add_definitions(-DHAVE_ANDROID_AUDIO)
      add_compile_definitions(HAVE_ANDROID_AUDIO)
      message(STATUS "Android audio support enabled with OpenSL ES")
    else()
      message(WARNING "OpenSL ES not found. Building without Android audio.")
      set(ENABLE_SOUND OFF)
    endif()
  endif()

  # Android-specific compiler flags
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -ffunction-sections -fdata-sections")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -ffunction-sections -fdata-sections")

  # Android-specific linker flags
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,--strip-all")

else()
  # Linux/Desktop packages
  find_package(ZLIB REQUIRED)
  find_package(PNG REQUIRED)
  find_package(SDL2 REQUIRED)
  find_package(OpenGL REQUIRED)

  # SDL_mixer support
  if(ENABLE_SOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

    if(SDL2_MIXER_FOUND)
      message(STATUS "Found SDL2_mixer: ${SDL2_MIXER_LIBRARIES}")
      add_definitions(-DHAVE_SDL_SOUND)
      add_compile_definitions(HAVE_SDL_SOUND)
    else()
      message(WARNING "SDL2_mixer not found. Building without sound support.")
      set(ENABLE_SOUND OFF)
    endif()
  endif()

  # SDL_ttf support
  if(ENABLE_TTF)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

    if(SDL2_TTF_FOUND)
      message(STATUS "Found SDL2_ttf: ${SDL2_TTF_LIBRARIES}")
      add_definitions(-DHAVE_SDL_TTF)
      add_compile_definitions(HAVE_SDL_TTF)
    else()
      message(WARNING "SDL2_ttf not found. Building without TTF support.")
      set(ENABLE_TTF OFF)
    endif()

      # Add SDL2_image support
      pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

    if(SDL2_IMAGE_FOUND)
      message(STATUS "Found SDL2_image: ${SDL2_IMAGE_LIBRARIES}")
      add_definitions(-DHAVE_SDL_IMAGE)
      add_compile_definitions(HAVE_SDL_IMAGE)
    else()
      message(WARNING "SDL2_ttf not found. Building without TTF support.")
      set(ENABLE_TTF OFF)
    endif()
  endif()

  # libxmp support for IT file playback
  if(ENABLE_XMP)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBXMP REQUIRED libxmp)

    if(LIBXMP_FOUND)
        message(STATUS "Found libxmp: ${LIBXMP_LIBRARIES}")
        add_definitions(-DHAVE_LIBXMP)
        add_compile_definitions(HAVE_LIBXMP)
        include_directories(${LIBXMP_INCLUDE_DIRS})
        link_directories(${LIBXMP_LIBRARY_DIRS})

        # Add the library to the target
        target_link_libraries(nebu_audio ${LIBXMP_LIBRARIES})

        # Check for xmp.h
        if(EXISTS "${LIBXMP_INCLUDE_DIRS}/xmp.h")
            message(STATUS "Found xmp.h in: ${LIBXMP_INCLUDE_DIRS}")
        else()
            message(WARNING "Could not find xmp.h. IT file support may not work.")
        endif()
    else()
        message(WARNING "libxmp not found. Building without IT file support.")
        set(ENABLE_XMP OFF)
    endif()
  endif()
endif()

# Create the cmake directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cmake)

# Generate config.h.in file based on platform
if(BUILD_ANDROID)
  file(WRITE ${CMAKE_BINARY_DIR}/cmake/config.h.in
"/* GLTron Configuration Header */
#ifndef CONFIG_H
#define CONFIG_H

/* Version information */
#define VERSION \"0.7.1\"
#define PACKAGE \"gltron\"

/* Platform detection */
#define ANDROID 1
#define __ANDROID__ 1
#define IS_ANDROID 1

/* Feature flags */
#define USE_SCRIPTING 0
#define USE_OPENGL 1

/* Path separator */
#define PATH_SEPARATOR '/'

/* Debug macros */
#include <stdio.h>
#ifdef ANDROID_LOG_H_AVAILABLE
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, \"GLTron\", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, \"GLTron\", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, \"GLTron\", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, \"GLTron\", __VA_ARGS__)
#else
#define LOGI(...) printf(\"[INFO] \" __VA_ARGS__); printf(\"\\n\")
#define LOGE(...) fprintf(stderr, \"[ERROR] \" __VA_ARGS__); fprintf(stderr, \"\\n\")
#define LOGD(...) printf(\"[DEBUG] \" __VA_ARGS__); printf(\"\\n\")
#define LOGW(...) printf(\"[WARN] \" __VA_ARGS__); printf(\"\\n\")
#endif

/* Platform-specific options */
#cmakedefine LOCAL_DATA
#cmakedefine HAVE_ANDROID_AUDIO
#cmakedefine ANDROID

#endif /* CONFIG_H */"
  )
else()
  file(WRITE ${CMAKE_BINARY_DIR}/cmake/config.h.in
"/* GLTron Configuration Header */
#ifndef CONFIG_H
#define CONFIG_H

/* Version information */
#define VERSION \"0.7.1\"
#define PACKAGE \"gltron\"

/* Platform detection */
#define ANDROID 0
#define __ANDROID__ 0
#define IS_ANDROID 0

/* Feature flags */
#define USE_SCRIPTING 0
#define USE_OPENGL 1

/* Path separator */
#ifdef _WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

/* Debug macros */
#include <stdio.h>
#define LOGI(...) printf(\"[INFO] \" __VA_ARGS__); printf(\"\\n\")
#define LOGE(...) fprintf(stderr, \"[ERROR] \" __VA_ARGS__); fprintf(stderr, \"\\n\")
#define LOGD(...) printf(\"[DEBUG] \" __VA_ARGS__); printf(\"\\n\")
#define LOGW(...) printf(\"[WARN] \" __VA_ARGS__); printf(\"\\n\")

/* Platform-specific options */
#cmakedefine LOCAL_DATA
#cmakedefine HAVE_SDL_SOUND
#cmakedefine HAVE_SDL_TTF
#cmakedefine HAVE_LIBXMP

#endif /* CONFIG_H */"
  )
endif()

# Generate configuration header
configure_file(
  ${CMAKE_BINARY_DIR}/cmake/config.h.in
  ${CMAKE_BINARY_DIR}/config.h
)

# Create platform-specific gltron-config.h
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/include)
if(BUILD_ANDROID)
  file(WRITE ${CMAKE_BINARY_DIR}/src/include/gltron-config.h
"/* Android-specific configuration for GLtron */
#ifndef GLTRON_CONFIG_H
#define GLTRON_CONFIG_H

/* Directory paths for Android */
#define PREF_DIR \"/data/data/com.gltron.android/files/\"
#define SNAP_DIR \"/data/data/com.gltron.android/files/snapshots/\"
#define DATA_DIR \"/android_asset/\"
#define VERSION \"0.7.1-android\"

/* Android-specific definitions */
#define ANDROID_PACKAGE \"com.gltron.android\"

/* Android platform macros */
#define ANDROID 1
#define __ANDROID__ 1
#define IS_ANDROID 1

#endif /* GLTRON_CONFIG_H */"
  )
else()
  file(WRITE ${CMAKE_BINARY_DIR}/src/include/gltron-config.h
"/* Linux/Desktop configuration for GLtron */
#ifndef GLTRON_CONFIG_H
#define GLTRON_CONFIG_H

/* Directory paths for Linux */
#define PREF_DIR \"~/.gltronrc\"
#define SNAP_DIR \"~/gltron-snapshots\"
#define DATA_DIR \"/usr/local/share/gltron\"
#define VERSION \"0.7.1\"

/* Platform-specific function replacements */
#ifdef __APPLE__
  /* Mac OS X doesn't have sinf and others in older versions */
  #define sinf(x) sin(x)
  #define cosf(x) cos(x)
  #define sqrtf(x) sqrt(x)
  #define tanf(x) tan(x)
  #define acosf(x) acos(x)
#endif

/* Non-Android platform */
#define ANDROID 0
#define __ANDROID__ 0
#define IS_ANDROID 0

#endif /* GLTRON_CONFIG_H */"
  )
endif()

# Create android_config.h if building for Android
if(BUILD_ANDROID)
  file(WRITE ${CMAKE_BINARY_DIR}/src/include/android_config.h
"/* Android-specific configuration for GLtron */
#ifndef ANDROID_CONFIG_H
#define ANDROID_CONFIG_H

/* Android-specific configuration */
#define ANDROID 1
#define __ANDROID__ 1
#define IS_ANDROID 1

/* Android-specific settings */
#define ANDROID_PACKAGE \"com.gltron.android\"
#define ANDROID_MIN_API ${ANDROID_MIN_API}

/* Android ABI settings */
#define ANDROID_ABI \"${ANDROID_ABI}\"

/* Android platform settings */
#define ANDROID_PLATFORM \"${ANDROID_PLATFORM}\"

#endif /* ANDROID_CONFIG_H */"
  )
endif()

# For non-Android builds
if(NOT BUILD_ANDROID)
  file(WRITE ${CMAKE_BINARY_DIR}/src/include/platform_config.h
"/* Platform compatibility header for Linux */
#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

/* Include the generated config */
#include \"config.h\"
#include \"gltron-config.h\"

/* Linux-specific includes */
#include <SDL2/SDL.h>
#include <GL/gl.h>

/* Linux-specific logging macros */
#include <stdio.h>
#define LOGI(fmt, ...) printf(\"[INFO] \" fmt \"\\n\", ##__VA_ARGS__)
#define LOGE(fmt, ...) fprintf(stderr, \"[ERROR] \" fmt \"\\n\", ##__VA_ARGS__)
#define LOGD(fmt, ...) printf(\"[DEBUG] \" fmt \"\\n\", ##__VA_ARGS__)
#define LOGW(fmt, ...) printf(\"[WARN] \" fmt \"\\n\", ##__VA_ARGS__)
#define LOG_TAG \"GLTron\"

#endif /* PLATFORM_CONFIG_H */"
  )
endif()

# Include directories for generated headers
include_directories(
  ${CMAKE_BINARY_DIR}/src/include
  ${CMAKE_BINARY_DIR}
)

# Common functions to reduce duplication across CMakeLists.txt files
function(add_common_includes target_name)
  target_include_directories(${target_name} PUBLIC
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/nebu/include
    ${CMAKE_SOURCE_DIR}/lua5/include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
  )
endfunction()

function(add_common_platform_definitions target_name)
  if(BUILD_ANDROID)
    target_compile_definitions(${target_name} PRIVATE
      ANDROID=1
      __ANDROID__=1
      IS_ANDROID=1
    )
  else()
    # Do not define ANDROID/__ANDROID__/IS_ANDROID on non-Android to avoid #ifdef pitfalls
  endif()

  if(LOCAL_DATA)
    target_compile_definitions(${target_name} PRIVATE LOCAL_DATA)
  endif()

  if(ENABLE_SOUND)
    if(BUILD_ANDROID)
      target_compile_definitions(${target_name} PRIVATE HAVE_ANDROID_AUDIO)
    else()
      target_compile_definitions(${target_name} PRIVATE HAVE_SDL_SOUND)
    endif()
  endif()

  if(ENABLE_TTF)
    target_compile_definitions(${target_name} PRIVATE HAVE_SDL_TTF)
  endif()
endfunction()

function(create_standard_library module_name)
  file(GLOB SOURCES "*.c" "*.cpp")
  file(GLOB HEADERS "*.h")

  if(SOURCES)
    add_library(${module_name} STATIC ${SOURCES} ${HEADERS})
    add_common_includes(${module_name})
    add_common_platform_definitions(${module_name})
    message(STATUS "[${module_name}] Created library with sources: ${SOURCES}")
  else()
    add_library(${module_name} INTERFACE)
    message(STATUS "[${module_name}] Created dummy library (no source files found)")
  endif()
endfunction()

function(add_common_sdl_linking target_name)
  if(NOT BUILD_ANDROID)
    target_link_libraries(${target_name} PUBLIC
      sdl_compat
      ${SDL2_LIBRARIES}
      ${SDL2_TTF_LIBRARIES}
    )
  endif()
endfunction()

# Create SDL compatibility library (Linux only)
if(NOT BUILD_ANDROID)
  set(SDL_COMPAT_CANDIDATES
      src/base/sdl_compat.c
      base/sdl_compat.c
  )

  set(SDL_COMPAT_SOURCES "")
  foreach(file ${SDL_COMPAT_CANDIDATES})
    if(EXISTS "${CMAKE_SOURCE_DIR}/${file}")
      list(APPEND SDL_COMPAT_SOURCES ${file})
    endif()
  endforeach()

  if(SDL_COMPAT_SOURCES)
    add_library(sdl_compat STATIC ${SDL_COMPAT_SOURCES})
    target_include_directories(sdl_compat PUBLIC
      ${CMAKE_BINARY_DIR}/src/include
      ${CMAKE_SOURCE_DIR}/src/include
      ${SDL2_INCLUDE_DIRS}
    )
    target_link_libraries(sdl_compat PUBLIC ${SDL2_LIBRARIES})
    message(STATUS "SDL compatibility library created with sources: ${SDL_COMPAT_SOURCES}")
  else()
    # Create a dummy SDL compatibility library
    add_library(sdl_compat INTERFACE)
    message(STATUS "SDL compatibility library created (dummy)")
  endif()
endif()

# Copy assets to build directory for testing
if(NOT BUILD_ANDROID)
  # List of asset directories to copy
  set(ASSET_DIRS art data levels music scripts sounds)

  foreach(asset_dir ${ASSET_DIRS})
    if(EXISTS "${CMAKE_SOURCE_DIR}/${asset_dir}")
      # Copy the entire directory
      file(COPY "${CMAKE_SOURCE_DIR}/${asset_dir}"
           DESTINATION "${CMAKE_BINARY_DIR}")
      message(STATUS "Copied asset directory: ${asset_dir}")
    endif()
  endforeach()

  # Also copy individual asset files if they exist
  set(ASSET_FILES README CREDITS COPYING)
  foreach(asset_file ${ASSET_FILES})
    if(EXISTS "${CMAKE_SOURCE_DIR}/${asset_file}")
      file(COPY "${CMAKE_SOURCE_DIR}/${asset_file}"
           DESTINATION "${CMAKE_BINARY_DIR}")
      message(STATUS "Copied asset file: ${asset_file}")
    endif()
  endforeach()
endif()

# Add subdirectories for modular build
#add_subdirectory(glew)
add_subdirectory(lua5)
add_subdirectory(nebu/audio)
add_subdirectory(nebu/base)
add_subdirectory(nebu/filesystem)
add_subdirectory(nebu/input)
add_subdirectory(nebu/scripting)
add_subdirectory(nebu/system)
add_subdirectory(nebu/video)
add_subdirectory(src)

# Add android-specific module if building for Android
if(BUILD_ANDROID)
  if(EXISTS "${CMAKE_SOURCE_DIR}/android/CMakeLists.txt")
    add_subdirectory(android)
    message(STATUS "Android-specific module added from android/CMakeLists.txt")
  else()
    message(WARNING "Android build detected but android/CMakeLists.txt not found")
  endif()
endif()

# Summary message
message(STATUS "\n=== GLTron Modular Configuration Summary ===")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Android Build: ${BUILD_ANDROID}")
if(BUILD_ANDROID)
  message(STATUS "Android ABI: ${ANDROID_ABI}")
  message(STATUS "Android Platform: ${ANDROID_PLATFORM}")
  message(STATUS "Android NDK: ${ANDROID_NDK}")
endif()
message(STATUS "Local Data: ${LOCAL_DATA}")
message(STATUS "Sound Support: ${ENABLE_SOUND}")
message(STATUS "TTF Support: ${ENABLE_TTF}")
message(STATUS "=============================================")
