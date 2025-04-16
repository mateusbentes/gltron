# Main CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(gltron VERSION 0.71 LANGUAGES C CXX)

# Set policy CMP0079 to NEW to allow linking targets from different directories
if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

# Options
option(BUILD_ANDROID "Build for Android platform" OFF)
option(ENABLE_STATIC_LINKING "Enable static linking" OFF)
option(LOCAL_DATA "Use local data directory" ON)  # Changed to ON by default
option(ENABLE_SOUND "Enable sound support (requires SDL_sound)" ON)
option(USE_EMBEDDED_SCRIPTS "Use embedded Lua scripts" ON)  # Added option for embedded scripts

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
message(STATUS "USE_EMBEDDED_SCRIPTS is set to: ${USE_EMBEDDED_SCRIPTS}")

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

if(USE_EMBEDDED_SCRIPTS)
  add_definitions(-DUSE_EMBEDDED_SCRIPTS)
  # Also add it as a global compile definition to ensure it's passed to all targets
  add_compile_definitions(USE_EMBEDDED_SCRIPTS)
  message(STATUS "USE_EMBEDDED_SCRIPTS definition added")
endif()

# Find required packages
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

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
#cmakedefine USE_EMBEDDED_SCRIPTS

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

# Create embedded_scripts.h
file(WRITE ${CMAKE_BINARY_DIR}/src/include/scripting/embedded_scripts.h
"/* 
 * Header file for embedded scripts
 */
#ifndef EMBEDDED_SCRIPTS_H
#define EMBEDDED_SCRIPTS_H

#ifdef __cplusplus
extern \"C\" {
#endif

/* Structure to hold embedded script data */
typedef struct {
    const char* name;    /* Script filename */
    const char* content; /* Script content */
} EmbeddedScript;

/* Function to get an embedded script by name */
const char* get_embedded_script(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* EMBEDDED_SCRIPTS_H */
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

  # Only copy scripts if not using embedded scripts
  if(NOT USE_EMBEDDED_SCRIPTS)
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

# Resources - only add if not using embedded scripts
if(NOT USE_EMBEDDED_SCRIPTS)
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
  ${CMAKE_SOURCE_DIR}/src/game/init_stubs.c  # Add this line
)
target_include_directories(callbacks PUBLIC
  ${CMAKE_BINARY_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/src/include  # Ensure the source directory is included
)

# Main executable
add_executable(gltron ${CMAKE_SOURCE_DIR}/src/gltron.c)

# Add LOCAL_DATA definition specifically to the gltron target
target_compile_definitions(gltron PRIVATE LOCAL_DATA)

# Add USE_EMBEDDED_SCRIPTS definition if enabled
if(USE_EMBEDDED_SCRIPTS)
  target_compile_definitions(gltron PRIVATE USE_EMBEDDED_SCRIPTS)
endif()

# Find Lua (optional)
find_package(Lua)
if(LUA_FOUND)
  message(STATUS "Found Lua: ${LUA_LIBRARIES}")
else()
  message(STATUS "Lua not found, using stub implementations")
  # Set empty variables to avoid errors
  set(LUA_INCLUDE_DIR "")
  set(LUA_LIBRARIES "")
endif()

# Link libraries - IMPORTANT: Order matters for static linking
target_link_libraries(gltron PRIVATE
  sdl_compat  # Add the SDL compatibility library first
  lua5  # For luaL_loadstring - move this before nebu_scripting
  lualib5  # For lua_pcall - move this before nebu_scripting
  nebu_system  # Add nebu_system before nebu_video
  nebu_video  # Add nebu_video before nebu_scripting
  nebu_input  # Add nebu_input before nebu_scripting
  nebu_scripting  # For scripting_GetLuaState - move this before game
  nebu_filesystem  # For nebu_FS_Test - move this before game
  nebu_base  # For basic functions - move this before game
  nebu_audio  # Move nebu_audio up in the linking order
  callbacks  # Add this line to link against the callbacks library
  game
  input
  audio
  video
  configuration
  base
  filesystem
  lib3ds  # Add lib3ds library
  ${GLEW_LIBRARIES}
  ${PNG_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${SDL2_LIBRARIES}
  # Add dl library for dynamic loading on Unix-like systems
  $<$<NOT:$<PLATFORM_ID:Windows>>:dl>
)

# Conditionally add Lua libraries if found
if(LUA_FOUND)
  target_link_libraries(gltron PRIVATE ${LUA_LIBRARIES})
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

  # Only copy scripts if not using embedded scripts
  if(NOT USE_EMBEDDED_SCRIPTS)
    add_custom_command(TARGET gltron POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_SOURCE_DIR}/scripts ${CMAKE_BINARY_DIR}/scripts
      COMMENT "Copying scripts to build directory"
    )
  endif()
endif()

# Installation rules - only install resources if not using embedded scripts
install(TARGETS gltron DESTINATION bin)
install(DIRECTORY art/ DESTINATION share/gltron/art)
install(DIRECTORY data/ DESTINATION share/gltron/data)
install(DIRECTORY music/ DESTINATION share/gltron/music)
install(DIRECTORY levels/ DESTINATION share/gltron/levels)
install(DIRECTORY sounds/ DESTINATION share/gltron/sounds)
if(NOT USE_EMBEDDED_SCRIPTS)
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
  COMMAND ${CMAKE_COMMAND} -E echo "USE_EMBEDDED_SCRIPTS is set to: ${USE_EMBEDDED_SCRIPTS}"
  COMMAND ${CMAKE_COMMAND} -E echo "Build type is: ${CMAKE_BUILD_TYPE}"
  COMMAND ${CMAKE_COMMAND} -E echo "Resources should be in: ${CMAKE_BINARY_DIR}/scripts"
  COMMAND ls -la ${CMAKE_BINARY_DIR}/scripts
)
