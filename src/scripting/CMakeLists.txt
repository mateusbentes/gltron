# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Ensure the source file containing scripting_SetLuaState is included
# Double-check the directory for the correct source file

# Create the library
add_library(nebu_scripting STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(nebu_scripting PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/lua5/include
  ${SDL2_INCLUDE_DIRS}
)

# Link with Lua libraries
target_link_libraries(nebu_scripting PUBLIC
  sdl_compat
  lua5
  lualib5
  ${SDL2_LIBRARIES}
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_scripting PRIVATE ANDROID)
endif()
