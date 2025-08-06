# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(audio STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(audio PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(audio PUBLIC
  nebu_filesystem
  nebu_audio
  sdl_compat
  ${SDL2_LIBRARIES}
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(audio PRIVATE ANDROID)
endif()
