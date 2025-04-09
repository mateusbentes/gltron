# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Make sure touch_interface.c is included
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/touch_interface.c")

# Remove level.c since its functions are duplicated in game_level.c
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/level.c")

# Create the library
add_library(game STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(game PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/lua5/include
)

# Link libraries
target_link_libraries(game PUBLIC
  nebu_filesystem
  nebu_scripting
  sdl_compat
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(game PRIVATE ANDROID)
endif()
