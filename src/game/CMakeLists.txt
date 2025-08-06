# src/game/CMakeLists.txt
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# Remove scripting_helpers.c from the build
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/scripting_helpers.c")

add_library(game STATIC ${SOURCES} ${HEADERS})
add_common_includes(game)
add_common_platform_definitions(game)

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
)

add_common_sdl_linking(game)