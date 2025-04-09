# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the libraries
add_library(lua STATIC ${SOURCES} ${HEADERS})
add_library(lualib STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(lua PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(lualib PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add necessary compile definitions
target_compile_definitions(lua PRIVATE LUA_ANSI)
target_compile_definitions(lualib PRIVATE LUA_ANSI)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(lua PRIVATE ANDROID)
  target_compile_definitions(lualib PRIVATE ANDROID)
endif()

# On Unix-like systems, we need to link with math library
if(UNIX)
  target_link_libraries(lua PRIVATE m)
  target_link_libraries(lualib PRIVATE m)
endif()
