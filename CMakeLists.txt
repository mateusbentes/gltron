# Main CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(gltron VERSION 0.71 LANGUAGES C CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Set policy CMP0079 to NEW to allow linking targets from different directories
if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

# Options
option(BUILD_ANDROID "Build for Android platform" OFF)
option(ENABLE_STATIC_LINKING "Enable static linking" OFF)
option(LOCAL_DATA "Use local data directory" ON)  # Changed to ON by default
option(ENABLE_SOUND "Enable sound support (requires SDL_sound)" ON)
option(USE_SCRIPTING "Use Lua scripting" OFF)  # Added option for use scripting

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Platform detection
if(APPLE)
  set(MACOSX TRUE)
elseif(WIN32)
  # WIN32 is already defined by CMake
elseif(UNIX AND NOT APPLE)
  set(LINUX TRUE)
endif()

# Force LOCAL_DATA to be ON during development
set(LOCAL_DATA ON CACHE BOOL "Use local data directory" FORCE)
message(STATUS "LOCAL_DATA is set to: ${LOCAL_DATA}")
message(STATUS "USE_SCRIPTING is set to: ${USE_SCRIPTING}")

# Add compiler definitions
if(BUILD_ANDROID)
  add_definitions(-DANDROID)
endif()

if(LOCAL_DATA)
  add_definitions(-DLOCAL_DATA)
  # Also add it as a global compile definition to ensure it's passed to all targets
  add_compile_definitions(LOCAL_DATA)
  message(STATUS "LOCAL_DATA definition added")
endif()

if(USE_SCRIPTING)
  add_definitions(-DUSE_SCRIPTING)
  # Also add it as a global compile definition to ensure it's passed to all targets
  add_compile_definitions(USE_SCRIPTING)
  message(STATUS "USE_SCRIPTING definition added")
endif()

# Find required packages
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

# SDL_mixer support
find_package(SDL2_mixer REQUIRED)

if(SDL2_MIXER_FOUND)
  message(STATUS "Found SDL2_mixer: ${SDL2_MIXER_LIBRARIES}")
  include_directories(${SDL2_MIXER_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "SDL2_mixer not found. Install it with your package manager.")
endif()

# Directly specify the paths to the lib3ds library and its include directory
set(LIB3DS_LIBRARY lib3ds)
set(LIB3DS_INCLUDE_DIR lib3ds)

# Make sure we have the GLEW::GLEW target
if(NOT TARGET GLEW::GLEW)
  add_library(GLEW::GLEW UNKNOWN IMPORTED)
  set_target_properties(GLEW::GLEW PROPERTIES
    IMPORTED_LOCATION "${GLEW_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}")
endif()

if(NOT BUILD_ANDROID)
  find_package(OpenGL REQUIRED)

  # Find SDL_sound if sound is enabled
  if(ENABLE_SOUND)
    find_path(SDL_SOUND_INCLUDE_DIR SDL_sound.h
      HINTS
      ENV SDL_SOUNDDIR
      ENV SDLSOUNDDIR
      PATH_SUFFIXES include/SDL include SDL SDL2
      PATHS
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local/include/SDL
      /usr/include/SDL
      /usr/local/include
      /usr/include
      /sw/include/SDL # Fink
      /sw/include
      /opt/local/include/SDL # DarwinPorts
      /opt/local/include
      /opt/csw/include/SDL # Blastwave
      /opt/csw/include
      /opt/include/SDL
      /opt/include
    )

    find_library(SDL_SOUND_LIBRARY
      NAMES SDL_sound
      HINTS
      ENV SDL_SOUNDDIR
      ENV SDLSOUNDDIR
      PATH_SUFFIXES lib64 lib
      PATHS
      ~/Library/Frameworks
      /Library/Frameworks
      /usr/local
      /usr
      /sw
      /opt/local
      /opt/csw
      /opt
    )

    if(SDL_SOUND_INCLUDE_DIR AND SDL_SOUND_LIBRARY)
      include_directories(${SDL_SOUND_INCLUDE_DIR})
      message(STATUS "Found SDL_sound: ${SDL_SOUND_LIBRARY}")
      add_definitions(-DHAVE_SDL_SOUND)
    else()
      message(WARNING "SDL_sound not found. Building without sound support.")
      set(ENABLE_SOUND OFF)
    endif()
  endif()
endif()

# Create the cmake directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cmake)

# Generate config.h.in file
file(WRITE ${CMAKE_BINARY_DIR}/cmake/config.h.in
"#ifndef CONFIG_H
#define CONFIG_H

#cmakedefine LOCAL_DATA
#cmakedefine HAVE_SDL_SOUND
#cmakedefine ANDROID
#cmakedefine USE_SCRIPTING

#endif /* CONFIG_H */
")

# Generate configuration header
configure_file(
  ${CMAKE_BINARY_DIR}/cmake/config.h.in
  ${CMAKE_BINARY_DIR}/config.h
)
include_directories(${CMAKE_BINARY_DIR})

# Create gltron-config.h
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/include)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/src/include/scripting)
file(WRITE ${CMAKE_BINARY_DIR}/src/include/gltron-config.h
"/* Platform-specific configuration for GLtron */
#ifndef GLTRON_CONFIG_H
#define GLTRON_CONFIG_H

/* Directory paths */
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

# Include directories - IMPORTANT: Order matters!
include_directories(
  ${CMAKE_BINARY_DIR}/src/include  # This should be first to ensure our headers are found first
  ${CMAKE_BINARY_DIR}  # For generated files like config.h
  ${CMAKE_SOURCE_DIR}/lua5/include
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/lib3ds/include  # Add lib3ds include directory
  ${CMAKE_SOURCE_DIR}/src/include  # Add the root directory to include path
  ${GLEW_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
  ${lib3ds_INCLUDE_DIRS}  # Add lib3ds include directories
)

# Create SDL compatibility library
add_library(sdl_compat STATIC
  ${CMAKE_SOURCE_DIR}/src/base/sdl_compat.c
)
target_include_directories(sdl_compat PUBLIC
  ${CMAKE_BINARY_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/src/include  # Ensure the source directory is included
  ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(sdl_compat PUBLIC ${SDL2_LIBRARIES})

# Copy resources to build directory for LOCAL_DATA mode
# Do this before building any targets to ensure resources are available
if(LOCAL_DATA)
  # Create directories
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/art)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/music)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/levels)
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/sounds)

  # Copy resources immediately during configure step
  file(COPY ${CMAKE_SOURCE_DIR}/art/ DESTINATION ${CMAKE_BINARY_DIR}/art)
  file(COPY ${CMAKE_SOURCE_DIR}/data/ DESTINATION ${CMAKE_BINARY_DIR}/data)
  file(COPY ${CMAKE_SOURCE_DIR}/music/ DESTINATION ${CMAKE_BINARY_DIR}/music)
  file(COPY ${CMAKE_SOURCE_DIR}/levels/ DESTINATION ${CMAKE_BINARY_DIR}/levels)
  file(COPY ${CMAKE_SOURCE_DIR}/sounds/ DESTINATION ${CMAKE_BINARY_DIR}/sounds)

  # Only copy scripts if using USE_SCRIPTING
  if(USE_SCRIPTING)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/scripts)
    file(COPY ${CMAKE_SOURCE_DIR}/scripts/ DESTINATION ${CMAKE_BINARY_DIR}/scripts)
  endif()

  message(STATUS "Resources copied to build directory")
endif()

# Add subdirectories for all libraries
add_subdirectory(lua5)
add_subdirectory(lib3ds)
add_subdirectory(nebu)
add_subdirectory(src)

# Resources - only add if using USE_SCRIPTING
if(USE_SCRIPTING)
  add_subdirectory(art)
  add_subdirectory(data)
  add_subdirectory(music)
  add_subdirectory(scripts)
  add_subdirectory(levels)
  add_subdirectory(sounds)
endif()

# Create a library for the callback functions
add_library(callbacks STATIC
  ${CMAKE_SOURCE_DIR}/src/game/callbacks.c
  ${CMAKE_SOURCE_DIR}/src/game/init_stubs.c
)
target_include_directories(callbacks PUBLIC
  ${CMAKE_BINARY_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/src/include  # Ensure the source directory is included
)

# Main executable
add_executable(gltron ${CMAKE_SOURCE_DIR}/src/gltron.c)

# Add LOCAL_DATA definition specifically to the gltron target
target_compile_definitions(gltron PRIVATE LOCAL_DATA)

# Add USE_SCRIPTING definition if enabled
if(NOT USE_SCRIPTING)
  target_compile_definitions(gltron PRIVATE USE_SCRIPTING)
endif()

# Find Lua (optional)
if(USE_SCRIPTING)
  find_package(Lua REQUIRED)
  if(LUA_FOUND)
    message(STATUS "Found Lua: ${LUA_LIBRARIES}")
  else()
    message(FATAL_ERROR "Lua not found. Please install Lua and try again.")
  endif()
endif()

# Link libraries - IMPORTANT: Order matters for static linking
target_link_libraries(gltron PRIVATE
  sdl_compat  # SDL compatibility wrapper

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
  ${SDL2_MIXER_LIBRARIES}

  # Dynamic loading (Unix only)
  $<$<NOT:$<PLATFORM_ID:Windows>>:dl>
)

# Link Lua-related libraries only when scripting is enabled
if(USE_SCRIPTING)
  target_link_libraries(gltron PRIVATE
    lua5         # Required for luaL_loadstring
    lualib5      # Required for lua_pcall
    nebu_scripting  # Script interface (must be linked after Lua libs)
    ${LUA_LIBRARIES}
  )
endif()

# Add math library only on non-Windows platforms
if(NOT WIN32)
  target_link_libraries(gltron PRIVATE m)
endif()

# Platform-specific configurations
if(BUILD_ANDROID)
  # Android-specific setup
  set(CMAKE_ANDROID_API 21)
  set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a arm64-v8a x86 x86_64)

  # Include Android-specific libraries
  find_library(ANDROID_LIB android)
  find_library(LOG_LIB log)
  find_library(EGL_LIB EGL)
  find_library(GLES_LIB GLESv2)

  target_link_libraries(gltron PRIVATE
    ${ANDROID_LIB}
    ${LOG_LIB}
    ${EGL_LIB}
    ${GLES_LIB}
  )
endif()

if(ENABLE_SOUND)
  target_link_libraries(gltron PRIVATE ${SDL_SOUND_LIBRARY})
endif()

# Math library is already added with target_link_libraries above

# Also copy resources as a post-build step to ensure they're updated if changed
if(LOCAL_DATA)
  add_custom_command(TARGET gltron POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/art ${CMAKE_BINARY_DIR}/art
    COMMENT "Copying art to build directory"
  )

  add_custom_command(TARGET gltron POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
    COMMENT "Copying data to build directory"
  )

  add_custom_command(TARGET gltron POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/music ${CMAKE_BINARY_DIR}/music
    COMMENT "Copying music to build directory"
  )

  add_custom_command(TARGET gltron POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/levels ${CMAKE_BINARY_DIR}/levels
    COMMENT "Copying levels to build directory"
  )

  add_custom_command(TARGET gltron POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/sounds ${CMAKE_BINARY_DIR}/sounds
    COMMENT "Copying sounds to build directory"
  )

  # Only copy scripts if not using USE_SCRIPTING
  if(NOT USE_SCRIPTING)
    add_custom_command(TARGET gltron POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/scripts ${CMAKE_BINARY_DIR}/scripts
      COMMENT "Copying scripts to build directory"
    )
  endif()
endif()

# Installation rules - only install resources if not using USE_SCRIPTING
install(TARGETS gltron DESTINATION bin)
install(DIRECTORY art/ DESTINATION share/gltron/art)
install(DIRECTORY data/ DESTINATION share/gltron/data)
install(DIRECTORY music/ DESTINATION share/gltron/music)
install(DIRECTORY levels/ DESTINATION share/gltron/levels)
install(DIRECTORY sounds/ DESTINATION share/gltron/sounds)
if(USE_SCRIPTING)
  install(DIRECTORY scripts/ DESTINATION share/gltron/scripts)
endif()

# Generate tags (similar to the original Makefile.am)
add_custom_target(tags
  COMMAND find . -name '*.c' -or -name '*.cpp' -or -name '*.h' -or -name '*.lua' | xargs etags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add a custom target to verify LOCAL_DATA is defined
add_custom_target(verify_local_data
  COMMAND ${CMAKE_COMMAND} -E echo "LOCAL_DATA is set to: ${LOCAL_DATA}"
  COMMAND ${CMAKE_COMMAND} -E echo "Resources should be in: ${CMAKE_BINARY_DIR}/scripts"
  COMMAND ls -la ${CMAKE_BINARY_DIR}/scripts
)

if(NOT USE_SCRIPTING)
  message(STATUS "NOT USE_SCRIPTING is set to: TRUE")
else()
  message(STATUS "NOT USE_SCRIPTING is set to: FALSE")
endif()