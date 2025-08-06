# FindSDL2_mixer.cmake
# Locate SDL2_mixer library
#
# This module defines:
#  SDL2_MIXER_FOUND - system has SDL2_mixer
#  SDL2_MIXER_INCLUDE_DIRS - the SDL2_mixer include directory
#  SDL2_MIXER_LIBRARIES - the SDL2_mixer libraries

find_path(SDL2_MIXER_INCLUDE_DIR SDL_mixer.h
  HINTS ENV SDL2MIXERDIR
  PATH_SUFFIXES include/SDL2 include
)

find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer
  HINTS ENV SDL2MIXERDIR
  PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SDL2_mixer DEFAULT_MSG
  SDL2_MIXER_INCLUDE_DIR SDL2_MIXER_LIBRARY
)

if(SDL2_MIXER_FOUND)
  set(SDL2_MIXER_LIBRARIES ${SDL2_MIXER_LIBRARY})
  set(SDL2_MIXER_INCLUDE_DIRS ${SDL2_MIXER_INCLUDE_DIR})
endif()