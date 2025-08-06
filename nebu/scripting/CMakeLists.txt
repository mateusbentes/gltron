# nebu/scripting/CMakeLists.txt
create_standard_library(nebu_scripting)

# Link with Lua libraries specific to nebu_scripting
target_link_libraries(nebu_scripting PUBLIC
  lua5
  lualib5
)

add_common_sdl_linking(nebu_scripting)