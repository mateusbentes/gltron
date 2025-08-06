# Main CMakeLists.txt for GLTron (Universal: Linux + Android)
cmake_minimum_required(VERSION 3.10)
project(gltron VERSION 0.71 LANGUAGES C CXX)

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
if(ANDROID)
  set(BUILD_ANDROID ON)
  message(STATUS "Detected Android platform")
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

# Add embedded Lua
if(USE_SCRIPTING)
  add_library(lua5 STATIC
    lua5/lapi.c
    lua5/lcode.c
    lua5/lctype.c
    lua5/ldebug.c
    lua5/ldo.c
    lua5/ldump.c
    lua5/lfunc.c
    lua5/lgc.c
    lua5/llex.c
    lua5/lmem.c
    lua5/lobject.c
    lua5/lopcodes.c
    lua5/lparser.c
    lua5/lstate.c
    lua5/lstring.c
    lua5/ltable.c
    lua5/ltm.c
    lua5/lundump.c
    lua5/lvm.c
    lua5/lzio.c
    lua5/lauxlib.c
    lua5/lbaselib.c
    lua5/ldblib.c
    lua5/liolib.c
    lua5/lmathlib.c
    lua5/loslib.c
    lua5/lstrlib.c
    lua5/ltablib.c
    lua5/loadlib.c
    lua5/linit.c
  )
  target_include_directories(lua5 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/lua5
  ${CMAKE_CURRENT_SOURCE_DIR}/lua5/include
)
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

  message(STATUS "Android libraries:")
  message(STATUS "  Android: ${ANDROID_LIB}")
  message(STATUS "  Log: ${LOG_LIB}")
  message(STATUS "  EGL: ${EGL_LIB}")
  message(STATUS "  GLESv2: ${GLES_LIB}")
  if(ENABLE_SOUND)
    message(STATUS "  OpenSLES: ${OPENSLES_LIB}")
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

  # lib3ds manual setup
  set(LIB3DS_LIBRARY lib3ds)
  set(LIB3DS_INCLUDE_DIR lib3ds)

  message(STATUS "Linux libraries found:")
  message(STATUS "  SDL2: ${SDL2_LIBRARIES}")
  message(STATUS "  OpenGL: ${OPENGL_LIBRARIES}")
  message(STATUS "  GLEW: ${GLEW_LIBRARIES}")
  message(STATUS "  PNG: ${PNG_LIBRARIES}")
  message(STATUS "  ZLIB: ${ZLIB_LIBRARIES}")
  if(ENABLE_SOUND)
    message(STATUS "  SDL2_mixer: ${SDL2_MIXER_LIBRARIES}")
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

# Include directories - IMPORTANT: Order matters!
include_directories(
  ${CMAKE_BINARY_DIR}/src/include  # Generated headers first
  ${CMAKE_BINARY_DIR}  # For config.h
)

# Platform-specific include directories
if(BUILD_ANDROID)
  include_directories(
    ${CMAKE_SOURCE_DIR}/include  # Android-specific headers
    ${CMAKE_SOURCE_DIR}/include/base
    ${CMAKE_SOURCE_DIR}/include/video
    ${CMAKE_SOURCE_DIR}/include/input
    ${CMAKE_SOURCE_DIR}/include/audio
    ${CMAKE_SOURCE_DIR}/include/game
    ${CMAKE_SOURCE_DIR}/include/filesystem
    ${CMAKE_SOURCE_DIR}/include/configuration
    ${CMAKE_SOURCE_DIR}/include/scripting
  )
else()
  # Linux - only include standard directories, avoid Android-specific ones
  include_directories(
    ${CMAKE_SOURCE_DIR}/src/include  # Standard GLTron headers
    ${CMAKE_SOURCE_DIR}/nebu/include  # Nebu library headers
    ${CMAKE_SOURCE_DIR}/lib3ds/include  # Add lib3ds include directory
    ${SDL2_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
  )
  if(ENABLE_SOUND)
    include_directories(${SDL2_MIXER_INCLUDE_DIRS})
  endif()
endif()

# Check which source files actually exist
function(check_source_files file_list result_var)
  set(existing_files "")
  foreach(file ${file_list})
    if(EXISTS "${CMAKE_SOURCE_DIR}/${file}")
      list(APPEND existing_files ${file})
    else()
      message(STATUS "Source file not found: ${file}")
    endif()
  endforeach()
  set(${result_var} ${existing_files} PARENT_SCOPE)
endfunction()

# Create SDL compatibility library (Linux only)
if(NOT BUILD_ANDROID)
  set(SDL_COMPAT_CANDIDATES
      src/base/sdl_compat.c
      base/sdl_compat.c
  )
  check_source_files("${SDL_COMPAT_CANDIDATES}" SDL_COMPAT_SOURCES)
  
  if(SDL_COMPAT_SOURCES)
    add_library(sdl_compat STATIC ${SDL_COMPAT_SOURCES})
    target_include_directories(sdl_compat PUBLIC
      ${CMAKE_BINARY_DIR}/src/include
      ${CMAKE_SOURCE_DIR}/src/include
      ${SDL2_INCLUDE_DIRS}
    )
    target_link_libraries(sdl_compat PUBLIC ${SDL2_LIBRARIES})
    message(STATUS "SDL compatibility library created")
  else()
    # Create a dummy SDL compatibility library
    add_library(sdl_compat INTERFACE)
    message(STATUS "SDL compatibility library created (dummy)")
  endif()
endif()

# Create modular libraries based on existing files
set(MODULE_DEFINITIONS
    "callbacks:src/game/callbacks.c,src/game/init_stubs.c,game/callbacks.c,game/init_stubs.c"
    "game:src/game/game.c,src/game/engine.c,src/game/level.c,game/game.c,game/engine.c,game/level.c"
    "input:src/input/input.c,input/input.c"
    "audio:src/audio/sound.c,src/audio/sound_stubs.c,audio/sound.c,audio/sound_stubs.c"
    "video:src/video/video.c,src/video/graphics.c,video/video.c,video/graphics.c"
    "configuration:src/configuration/settings.c,configuration/settings.c"
    "base:src/base/util.c,base/util.c"
    "filesystem:src/filesystem/path.c,src/filesystem/dirsetup.c,filesystem/path.c,filesystem/dirsetup.c"
    "nebu_system:nebu/base/nebu_system.c,nebu/system/system.c"
    "nebu_video:nebu/video/nebu_video.c,nebu/video/video.c"
    "nebu_input:nebu/input/nebu_input.c,nebu/input/input.c"
    "nebu_filesystem:nebu/filesystem/nebu_filesystem.c,nebu/filesystem/filesystem.c"
    "nebu_base:nebu/base/nebu_math.c,nebu/base/nebu_util.c,nebu/base/math.c,nebu/base/util.c"
    "nebu_audio:nebu/audio/nebu_audio.c,nebu/audio/audio.c"
)

# Create libraries for each module
foreach(module_def ${MODULE_DEFINITIONS})
  string(REPLACE ":" ";" module_parts ${module_def})
  list(GET module_parts 0 module_name)
  list(GET module_parts 1 module_files_str)
  string(REPLACE "," ";" module_files ${module_files_str})
  
  # Check which files exist for this module
  check_source_files("${module_files}" existing_module_files)
  
  if(existing_module_files)
    add_library(${module_name} STATIC ${existing_module_files})
    target_include_directories(${module_name} PUBLIC
      ${CMAKE_BINARY_DIR}/src/include
      ${CMAKE_SOURCE_DIR}/src/include
      ${CMAKE_SOURCE_DIR}/nebu/include
    )
    
    # Add platform-specific compile definitions
    if(BUILD_ANDROID)
      target_compile_definitions(${module_name} PRIVATE ANDROID)
      if(ENABLE_SOUND)
        target_compile_definitions(${module_name} PRIVATE HAVE_ANDROID_AUDIO)
      endif()
    else()
      if(ENABLE_SOUND)
        target_compile_definitions(${module_name} PRIVATE HAVE_SDL_SOUND)
      endif()
    endif()
    
    target_compile_definitions(${module_name} PRIVATE LOCAL_DATA)
    message(STATUS "Created library: ${module_name} with files: ${existing_module_files}")
  else()
    # Create a dummy library if no files found
    add_library(${module_name} INTERFACE)
    message(STATUS "Created dummy library: ${module_name} (no source files found)")
  endif()
endforeach()

# Try to create lib3ds library (Linux only)
if(NOT BUILD_ANDROID)
  set(LIB3DS_CANDIDATES
      lib3ds/lib3ds.c
      lib3ds/src/lib3ds.c
      src/lib3ds/lib3ds.c
  )
  check_source_files("${LIB3DS_CANDIDATES}" LIB3DS_SOURCES)
  
  if(LIB3DS_SOURCES)
    add_library(lib3ds STATIC ${LIB3DS_SOURCES})
    target_include_directories(lib3ds PUBLIC
      ${CMAKE_SOURCE_DIR}/lib3ds/include
      ${CMAKE_SOURCE_DIR}/lib3ds
    )
    message(STATUS "Created lib3ds library")
  else()
    # Create a dummy lib3ds library
    add_library(lib3ds INTERFACE)
    message(STATUS "Created dummy lib3ds library")
  endif()
endif()

# Main source files
set(MAIN_SOURCE_CANDIDATES
    gltron.c
    src/gltron.c
)

check_source_files("${MAIN_SOURCE_CANDIDATES}" MAIN_SOURCES)

if(NOT MAIN_SOURCES)
  message(FATAL_ERROR "No main source file found!")
endif()

# Platform-specific adaptations
if(BUILD_ANDROID)
  set(ANDROID_ADAPTATION_CANDIDATES
      android_gles_adapter.c
      android_input_adapter.c
      android_main.c
      android_audio_adapter.c
      android_audio_settings.c
      android_resolution_manager.c
      android_settings_menu.c
      scripting_stubs.c
      opengl_stubs.c
  )
  
  check_source_files("${ANDROID_ADAPTATION_CANDIDATES}" PLATFORM_ADAPTATIONS)
else()
  set(PLATFORM_ADAPTATIONS "")
endif()

# Combine all source files
set(ALL_SOURCES ${MAIN_SOURCES} ${PLATFORM_ADAPTATIONS})

# Create the main target
if(BUILD_ANDROID)
  # Android native library
  add_library(gltron_native SHARED ${ALL_SOURCES})
  set(TARGET_NAME gltron_native)
else()
  # Linux executable
  add_executable(gltron ${ALL_SOURCES})
  set(TARGET_NAME gltron)
endif()

# Add compile definitions to the target
target_compile_definitions(${TARGET_NAME} PRIVATE LOCAL_DATA)

if(BUILD_ANDROID)
  target_compile_definitions(${TARGET_NAME} PRIVATE ANDROID)
  if(ENABLE_SOUND)
    target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_ANDROID_AUDIO)
  endif()
else()
  if(ENABLE_SOUND)
    target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_SDL_SOUND)
  endif()
endif()

# Set target properties
set_target_properties(${TARGET_NAME} PROPERTIES
    C_STANDARD 99
    CXX_STANDARD 11
)

if(BUILD_ANDROID)
  set_target_properties(${TARGET_NAME} PROPERTIES
      POSITION_INDEPENDENT_CODE ON
  )
endif()

# Link libraries based on platform
if(BUILD_ANDROID)
  # Android libraries
  target_link_libraries(${TARGET_NAME} PRIVATE
      # Android-specific modules
      callbacks
      game
      input
      audio
      video
      configuration
      base
      filesystem
      
      # Android system libraries
      ${ANDROID_LIB}
      ${LOG_LIB}
      ${EGL_LIB}
      ${GLES_LIB}
  )
  
  if(ENABLE_SOUND AND OPENSLES_LIB)
    target_link_libraries(${TARGET_NAME} PRIVATE ${OPENSLES_LIB})
  endif()
  
else()
  # Linux/Desktop libraries - IMPORTANT: Order matters for static linking
  target_link_libraries(${TARGET_NAME} PRIVATE
      sdl_compat          # SDL compatibility wrapper
      
      callbacks          # Main menu and in-game callback logic
      
      game               # Main game logic
      input              # Rewritten input handling
      audio              # Rewritten audio handling
      video              # Rewritten video rendering
      configuration      # Custom config management module
      base               # Rewritten utility module
      filesystem         # Rewritten file handling
      
      nebu_system        # System utilities (must come before video/input)
      nebu_video         # Low-level video handling
      nebu_input         # Input device handling
      nebu_filesystem    # Filesystem operations
      nebu_base          # Basic math and utilities
      nebu_audio         # Legacy audio functions
      
      lib3ds             # 3D model loading support
      ${GLEW_LIBRARIES}
      ${PNG_LIBRARIES}
      ${ZLIB_LIBRARIES}
      ${OPENGL_LIBRARIES}
      ${SDL2_LIBRARIES}
  )
  
  if(ENABLE_SOUND)
    target_link_libraries(${TARGET_NAME} PRIVATE ${SDL2_MIXER_LIBRARIES})
  endif()
  
  # Dynamic loading (Unix only)
  if(NOT WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE dl)
  endif()
endif()

# Add math library
target_link_libraries(${TARGET_NAME} PRIVATE m)

# Copy resources to build directory for LOCAL_DATA mode
if(LOCAL_DATA)
  if(BUILD_ANDROID)
    # Create Android asset directories
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/art)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/data)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/music)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/assets/sounds)
    message(STATUS "Android asset directories created")
  else()
    # Create Linux directories
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/art)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/music)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/sounds)
    
    # Copy resources as post-build step
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/art ${CMAKE_BINARY_DIR}/art
        COMMENT "Copying art to build directory"
    )
    
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
        COMMENT "Copying data to build directory"
    )
    
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/music ${CMAKE_BINARY_DIR}/music
        COMMENT "Copying music to build directory"
    )
    
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/sounds ${CMAKE_BINARY_DIR}/sounds
        COMMENT "Copying sounds to build directory"
    )
    
    message(STATUS "Linux resource directories created")
  endif()
endif()

# Installation rules
if(BUILD_ANDROID)
  install(TARGETS ${TARGET_NAME} 
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
  )
else()
  install(TARGETS ${TARGET_NAME} DESTINATION bin)
  # Install resources for Linux
  if(EXISTS ${CMAKE_SOURCE_DIR}/art)
    install(DIRECTORY art/ DESTINATION share/gltron/art)
  endif()
  if(EXISTS ${CMAKE_SOURCE_DIR}/data)
    install(DIRECTORY data/ DESTINATION share/gltron/data)
  endif()
  if(EXISTS ${CMAKE_SOURCE_DIR}/music)
    install(DIRECTORY music/ DESTINATION share/gltron/music)
  endif()
  if(EXISTS ${CMAKE_SOURCE_DIR}/sounds)
    install(DIRECTORY sounds/ DESTINATION share/gltron/sounds)
  endif()
endif()

# Generate tags for development
add_custom_target(tags
    COMMAND find . -name '*.c' -or -name '*.cpp' -or -name '*.h' | xargs etags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Create a verification target
add_custom_target(verify_build
    COMMAND ${CMAKE_COMMAND} -E echo "Build configuration:"
    COMMAND ${CMAKE_COMMAND} -E echo "  Platform: ${CMAKE_SYSTEM_NAME}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Android: ${BUILD_ANDROID}"
    COMMAND ${CMAKE_COMMAND} -E echo "  LOCAL_DATA: ${LOCAL_DATA}"
    COMMAND ${CMAKE_COMMAND} -E echo "  ENABLE_SOUND: ${ENABLE_SOUND}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Target: ${TARGET_NAME}"
)

# Summary
message(STATUS "")
message(STATUS "GLTron Configuration Summary:")
message(STATUS "=============================")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
if(BUILD_ANDROID)
message(STATUS "Android API Level: ${CMAKE_ANDROID_API}")
message(STATUS "Android ABI: ${CMAKE_ANDROID_ARCH_ABI}")
endif()
message(STATUS "Local Data: ${LOCAL_DATA}")
message(STATUS "Sound Support: ${ENABLE_SOUND}")
message(STATUS "")
message(STATUS "Features:")
if(BUILD_ANDROID)
message(STATUS "  ✓ Android OpenGL ES rendering")
message(STATUS "  ✓ Touch input system")
if(ENABLE_SOUND)
message(STATUS "  ✓ Android audio system (OpenSL ES)")
message(STATUS "  ✓ Audio settings and controls")
endif()
message(STATUS "  ✓ Resolution management")
message(STATUS "  ✓ Graphics quality settings")
message(STATUS "  ✓ Settings menu system")
else()
message(STATUS "  ✓ Desktop OpenGL rendering")
message(STATUS "  ✓ Keyboard/mouse input")
if(ENABLE_SOUND)
message(STATUS "  ✓ SDL audio system")
endif()
message(STATUS "  ✓ Modular library structure")
endif()
message(STATUS "")
if(BUILD_ANDROID)
message(STATUS "Target: libgltron_native.so")
else()
message(STATUS "Target: gltron executable")
endif()
message(STATUS "")

