cmake_minimum_required(VERSION 3.16)
project(openmpt_wrapper_android LANGUAGES C CXX)

# Expect variables provided by the outer script via -D:
# OPENMPT_SRC_DIR, OPENMPT_PREFIX, OPENMPT_INCDIR, OPENMPT_LIBDIR

if(NOT DEFINED OPENMPT_SRC_DIR)
  message(FATAL_ERROR "OPENMPT_SRC_DIR not set")
endif()
if(NOT DEFINED OPENMPT_PREFIX)
  message(FATAL_ERROR "OPENMPT_PREFIX not set")
endif()
if(NOT DEFINED OPENMPT_INCDIR)
  message(FATAL_ERROR "OPENMPT_INCDIR not set")
endif()
if(NOT DEFINED OPENMPT_LIBDIR)
  message(FATAL_ERROR "OPENMPT_LIBDIR not set")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Dependencies
set(PREFIX "${OPENMPT_PREFIX}")
set(INCDIR "${OPENMPT_INCDIR}")
set(LIBDIR "${OPENMPT_LIBDIR}")

include_directories(
  ${INCDIR}
  ${OPENMPT_SRC_DIR}
  ${OPENMPT_SRC_DIR}/libopenmpt
  ${OPENMPT_SRC_DIR}/include
  ${OPENMPT_SRC_DIR}/src
  ${OPENMPT_SRC_DIR}/common
  ${OPENMPT_SRC_DIR}/soundlib
)

link_directories(${LIBDIR})

# Try to use OpenMPT's own CMake if present inside the source tree
if(EXISTS "${OPENMPT_SRC_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "OpenMPT CMakeLists.txt exists in source; wrapper should not be used")
endif()

# Build a minimal static lib by compiling libopenmpt public API and required core sources.
# Note: This is a pragmatic subset known to work for typical libopenmpt static builds.
file(GLOB OPENMPT_API
  "${OPENMPT_SRC_DIR}/libopenmpt/libopenmpt_c.cpp"
)

# Core components (hand-picked minimal set, may need adjustments if upstream changes)
file(GLOB OPENMPT_COMMON
  "${OPENMPT_SRC_DIR}/common/*.cpp"
)
file(GLOB OPENMPT_SONGLIB
  "${OPENMPT_SRC_DIR}/soundlib/*.cpp"
)

add_library(openmpt_static STATIC
  ${OPENMPT_API}
  ${OPENMPT_COMMON}
  ${OPENMPT_SONGLIB}
)

# C++ standard
set_target_properties(openmpt_static PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)

# Link dependencies
# We keep the order to satisfy static linking
find_library(OGG_LIB ogg HINTS ${LIBDIR})
find_library(VORBIS_LIB vorbis HINTS ${LIBDIR})
find_library(VORBISFILE_LIB vorbisfile HINTS ${LIBDIR})
find_library(VORBISENC_LIB vorbisenc HINTS ${LIBDIR})
find_library(FLAC_LIB FLAC HINTS ${LIBDIR})
find_library(SNDFILE_LIB sndfile HINTS ${LIBDIR})
find_library(OPUS_LIB opus HINTS ${LIBDIR})

# Some builds may not include all, so link only found ones
set(DEPS_LIBS)
foreach(L IN LISTS OGG_LIB VORBIS_LIB VORBISFILE_LIB VORBISENC_LIB FLAC_LIB SNDFILE_LIB OPUS_LIB)
  if(${L})
    list(APPEND DEPS_LIBS ${${L}})
  endif()
endforeach()

target_link_libraries(openmpt_static PRIVATE ${DEPS_LIBS})

# Force non-Windows, non-MFC, portable build options suitable for Android
# Make sure these definitions override any defaults in headers
target_compile_definitions(openmpt_static PRIVATE
  MPT_WITH_ZLIB=0
  MPT_WITH_MPG123=0
  MPT_WITH_MFC=0
  MPT_OS_WINDOWS=0
  MPT_OS_ANDROID=1
  MPT_OS_LINUX=1
  MPT_COMPILER_MSVC=0
  MPT_BUILD_WINESUPPORT=0
  MPT_BUILD_FUZZERS=0
  MPT_WITH_SDL2=0
  MPT_WITH_PORTAUDIO=0
  MPT_WITH_PULSEAUDIO=0
  MPT_WITH_ALSA=0
  MPT_EXCEPTIONS_USE_WINDOWS_SEH=0
  LIBOPENMPT_BUILD=1
)

# Exclude any MFC/Windows-only sources if present in globs
list(FILTER OPENMPT_COMMON EXCLUDE REGEX ".*mfc.*\\.cpp$")
list(FILTER OPENMPT_COMMON EXCLUDE REGEX ".*win.*\\.cpp$")
list(FILTER OPENMPT_SONGLIB EXCLUDE REGEX ".*mfc.*\\.cpp$")
list(FILTER OPENMPT_SONGLIB EXCLUDE REGEX ".*win.*\\.cpp$")

# Install
install(TARGETS openmpt_static ARCHIVE DESTINATION ${LIBDIR} LIBRARY DESTINATION ${LIBDIR})

# Install headers
install(FILES
  ${OPENMPT_SRC_DIR}/libopenmpt/libopenmpt.h
  ${OPENMPT_SRC_DIR}/libopenmpt/libopenmpt.hpp
  DESTINATION ${INCDIR}/libopenmpt
)
