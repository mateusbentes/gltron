# Collect all source and header files
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# Remove scripting_helpers.c from the build
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/scripting_helpers.c")

# Create the library
add_library(game STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(game PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/lua5/include
)

# Only link Lua-related stuff if scripting is enabled
if(USE_SCRIPTING)
  target_link_libraries(game PUBLIC
    nebu_scripting
    lua5
    lualib5
  )
endif()

# Always link these
target_link_libraries(game PUBLIC
  nebu_filesystem
  nebu_input
  nebu_video
  nebu_base
  sdl_compat
)
