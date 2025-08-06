# Modular Main CMakeLists.txt for GLTron
cmake_minimum_required(VERSION 3.10)
project(gltron VERSION 0.71 LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Set policy CMP0079 to NEW to allow linking targets from different directories
if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

# Set OpenGL policy to avoid warnings
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

# Detect platform automatically
if(ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(BUILD_ANDROID ON)
  message(STATUS "Detected Android platform")
  message(STATUS "Android ABI: ${ANDROID_ABI}")
  message(STATUS "Android Platform: ${ANDROID_PLATFORM}")
else()
  set(BUILD_ANDROID OFF)
  message(STATUS "Detected Linux/Desktop platform")
endif()

# Options
option(ENABLE_STATIC_LINKING "Enable static linking" OFF)
option(LOCAL_DATA "Use local data directory" ON)
option(ENABLE_SOUND "Enable sound support" ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Platform detection
if(APPLE)
  set(MACOSX TRUE)
elseif(WIN32)
  # WIN32 is already defined by CMake
elseif(UNIX AND NOT APPLE AND NOT ANDROID)
  set(LINUX TRUE)
endif()

# Force LOCAL_DATA to be ON during development
set(LOCAL_DATA ON CACHE BOOL "Use local data directory" FORCE)
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "LOCAL_DATA is set to: ${LOCAL_DATA}")
message(STATUS "ENABLE_SOUND is set to: ${ENABLE_SOUND}")
message(STATUS "BUILD_ANDROID is set to: ${BUILD_ANDROID}")

# Add compiler definitions
if(BUILD_ANDROID)
  add_definitions(-DANDROID)
  add_compile_definitions(ANDROID)
endif()

if(LOCAL_DATA)
  add_definitions(-DLOCAL_DATA)
  add_compile_definitions(LOCAL_DATA)
  message(STATUS "LOCAL_DATA definition added")
endif()

# Platform-specific package finding
if(BUILD_ANDROID)
  # Android-specific setup
  if(NOT ANDROID_NDK)
    message(FATAL_ERROR "Android NDK not found. Please set ANDROID_NDK or use Android toolchain.")
  endif()

  set(CMAKE_ANDROID_API 24)

  if(NOT CMAKE_ANDROID_ARCH_ABI)
    set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
  endif()

  # Find Android native libraries
  find_library(ANDROID_LIB android)
  find_library(LOG_LIB log)
  find_library(EGL_LIB EGL)
  find_library(GLES_LIB GLESv2)

  if(ENABLE_SOUND)
    find_library(OPENSLES_LIB OpenSLES)
    if(OPENSLES_LIB)
      add_definitions(-DHAVE_ANDROID_AUDIO)
      add_compile_definitions(HAVE_ANDROID_AUDIO)
      message(STATUS "Android audio support enabled")
    else()
      message(WARNING "OpenSL ES not found. Building without Android audio.")
      set(ENABLE_SOUND OFF)
    endif()
  endif()

else()
  # Linux/Desktop packages
  find_package(ZLIB REQUIRED)
  find_package(PNG REQUIRED)
  find_package(SDL2 REQUIRED)
  find_package(OpenGL REQUIRED)

  # Try to find GLEW
  find_package(GLEW)
  if(NOT GLEW_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLEW REQUIRED glew)
  endif()

  # Ensure GLEW::GLEW target
  if(NOT TARGET GLEW::GLEW)
    add_library(GLEW::GLEW UNKNOWN IMPORTED)
    set_target_properties(GLEW::GLEW PROPERTIES
      IMPORTED_LOCATION "${GLEW_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}")
  endif()

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
endif()

# Create the cmake directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cmake)

# Generate config.h.in file
if(BUILD_ANDROID)
  file(WRITE ${CMAKE_BINARY_DIR}/cmake/config.h.in
"#ifndef CONFIG_H
#define CONFIG_H

#cmakedefine LOCAL_DATA
#cmakedefine HAVE_ANDROID_AUDIO
#cmakedefine ANDROID

#endif /* CONFIG_H */
")
else()
  file(WRITE ${CMAKE_BINARY_DIR}/cmake/config.h.in
"#ifndef CONFIG_H
#define CONFIG_H

#cmakedefine LOCAL_DATA
#cmakedefine HAVE_SDL_SOUND

#endif /* CONFIG_H */
")
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

#endif /* GLTRON_CONFIG_H */
")
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

#endif /* GLTRON_CONFIG_H */
")
endif()

# Create a platform compatibility header
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
#include <GL/glew.h>
#include <GL/gl.h>

/* Linux-specific logging macros */
#include <stdio.h>
#define LOGI(fmt, ...) printf(\"[INFO] \" fmt \"\\n\", ##__VA_ARGS__)
#define LOGE(fmt, ...) fprintf(stderr, \"[ERROR] \" fmt \"\\n\", ##__VA_ARGS__)
#define LOG_TAG \"GLTron\"

#endif /* PLATFORM_CONFIG_H */
")
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
  )
endfunction()

function(add_common_platform_definitions target_name)
  if(BUILD_ANDROID)
    target_compile_definitions(${target_name} PRIVATE ANDROID)
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
  target_link_libraries(${target_name} PUBLIC
    sdl_compat
    ${SDL2_LIBRARIES}
  )
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
add_subdirectory(glew)
add_subdirectory(lua5)
add_subdirectory(nebu/audio)
add_subdirectory(nebu/base)
add_subdirectory(nebu/filesystem)
add_subdirectory(nebu/input)
add_subdirectory(nebu/scripting)
add_subdirectory(nebu/system)
add_subdirectory(nebu/video)
add_subdirectory(src)

# Summary message
message(STATUS "\nGLTron Modular Configuration Summary:")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Local Data: ${LOCAL_DATA}")
message(STATUS "Sound Support: ${ENABLE_SOUND}")
