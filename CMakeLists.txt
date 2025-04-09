cmake_minimum_required(VERSION 3.10)
project(gltron VERSION 0.71 LANGUAGES C CXX)

# Options
option(BUILD_ANDROID "Build for Android platform" OFF)
option(ENABLE_STATIC_LINKING "Enable static linking" OFF)
option(LOCAL_DATA "Use local data directory" OFF)
option(ENABLE_SOUND "Enable sound support (requires SDL_sound)" ON)

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

if(BUILD_ANDROID)
  add_definitions(-DANDROID)
endif()

if(LOCAL_DATA)
  add_definitions(-DLOCAL_DATA)
endif()

# Find required packages
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

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

# Generate configuration header
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigHeader.cmake)

# Include directories
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lua5/include
  ${CMAKE_CURRENT_SOURCE_DIR}/nebu/include
  ${CMAKE_CURRENT_SOURCE_DIR}  # Add the root directory to include path
  ${GLEW_INCLUDE_DIRS}
  ${PNG_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
)

# Add subdirectories for all libraries
add_subdirectory(lua5)
add_subdirectory(lib3ds)
add_subdirectory(nebu)
add_subdirectory(src)

# Resources
add_subdirectory(art)
add_subdirectory(data)
add_subdirectory(music)
add_subdirectory(scripts)
add_subdirectory(levels)

# Main executable
add_executable(gltron ${CMAKE_CURRENT_SOURCE_DIR}/src/gltron.c)

# Link libraries - IMPORTANT: Order matters for static linking
target_link_libraries(gltron PRIVATE
  game
  input
  audio
  video
  configuration
  base
  filesystem
  nebu_input
  nebu_audio
  nebu_video
  nebu_scripting
  nebu_filesystem
  nebu_base
  lua
  lualib
  lib3ds
  ${GLEW_LIBRARIES}
  ${PNG_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${SDL2_LIBRARIES}
  # Add math library for Unix-like systems
  $<$<NOT:$<PLATFORM_ID:Windows>>:m>
  # Add dl library for dynamic loading on Unix-like systems
  $<$<NOT:$<PLATFORM_ID:Windows>>:dl>
)

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

# Installation rules
install(TARGETS gltron DESTINATION bin)
install(DIRECTORY art/ DESTINATION share/gltron/art)
install(DIRECTORY data/ DESTINATION share/gltron/data)
install(DIRECTORY music/ DESTINATION share/gltron/music)
install(DIRECTORY scripts/ DESTINATION share/gltron/scripts)
install(DIRECTORY levels/ DESTINATION share/gltron/levels)

# Generate tags (similar to the original Makefile.am)
add_custom_target(tags
  COMMAND find . -name '*.c' -or -name '*.cpp' -or -name '*.h' -or -name '*.lua' | xargs etags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
