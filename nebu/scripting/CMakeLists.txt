# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(nebu_scripting STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(nebu_scripting PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/lua5/include
)

# Link with Lua libraries
target_link_libraries(nebu_scripting PUBLIC
  lua
  lualib
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_scripting PRIVATE ANDROID)
endif()
