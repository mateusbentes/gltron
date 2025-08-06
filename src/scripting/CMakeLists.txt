# src/scripting/CMakeLists.txt
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

if(SOURCES)
  add_library(src_scripting STATIC ${SOURCES} ${HEADERS})
  add_common_includes(src_scripting)
  add_common_platform_definitions(src_scripting)
  
  # Link with Lua libraries specific to src_scripting
  target_link_libraries(src_scripting PUBLIC
    lua5
    lualib5
  )
  
  add_common_sdl_linking(src_scripting)
  message(STATUS "[src/scripting] Created src_scripting library with sources: ${SOURCES}")
else()
  add_library(src_scripting INTERFACE)
  message(STATUS "[src/scripting] Created dummy src_scripting library (no source files found)")
endif()