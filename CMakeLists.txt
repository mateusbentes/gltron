cmake_minimum_required(VERSION 3.10)
project(gltron VERSION 0.71 LANGUAGES C CXX)

# Options
option(BUILD_ANDROID "Build for Android platform" OFF)
option(ENABLE_STATIC_LINKING "Enable static linking" OFF)
option(LOCAL_DATA "Use local data directory" OFF)

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

# Find SDL2 package
if(NOT BUILD_ANDROID)
  find_package(SDL2 REQUIRED)
  # Add SDL2 include directories
  include_directories(${SDL2_INCLUDE_DIRS})
endif()

# Generate configuration header
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ConfigHeader.cmake)

# Include directories
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lua5/include
  ${CMAKE_CURRENT_SOURCE_DIR}/nebu/include
)

# Add subdirectories for all libraries
add_subdirectory(glew)
add_subdirectory(lib3ds)
add_subdirectory(lua5)
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

# Link libraries
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
  glew
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
else()
  # Desktop platform dependencies
  find_package(OpenGL REQUIRED)
  find_package(SDL2 REQUIRED)
  
  target_link_libraries(gltron PRIVATE
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
  )
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
