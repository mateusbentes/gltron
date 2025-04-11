file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# Remove scripting_helpers.c from the build
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/scripting_helpers.c")

# Add embedded_processing.c explicitly
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/embedded_processing.c")

# Create the library
add_library(game STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(game PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/lua5/include
)

# Link libraries - Order matters for static linking
target_link_libraries(game PUBLIC
  nebu_scripting  # For scripting_GetLuaState
  lua5  # For luaL_loadstring
  lualib5  # For lua_pcall
  nebu_filesystem  # For nebu_FS_Test
  nebu_input
  nebu_video
  nebu_base  # For basic functions
  sdl_compat
)
