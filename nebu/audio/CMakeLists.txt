# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(nebu_audio STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(nebu_audio PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${SDL_SOUND_INCLUDE_DIR}  # Add SDL_sound include directory
)

# Link against SDL_sound
if(NOT BUILD_ANDROID)
  target_link_libraries(nebu_audio PRIVATE ${SDL_SOUND_LIBRARY})
endif()

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_audio PRIVATE ANDROID)
endif()
