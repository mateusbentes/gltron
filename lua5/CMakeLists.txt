# Check if source directories exist and print their contents
message(STATUS "Checking lua5 source directories:")
message(STATUS "Current source dir: ${CMAKE_CURRENT_SOURCE_DIR}")

# List directories to help debug
file(GLOB LUA_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*")
foreach(dir ${LUA_DIRS})
  message(STATUS "Found directory/file: ${dir}")
endforeach()

# Try different paths for source files
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src")
  # Standard path
  file(GLOB LUA_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/lib")
    file(GLOB LUALIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/lib/*.c")
  else()
    # Try alternative paths for lib sources
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib")
      file(GLOB LUALIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.c")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/lualib")
      file(GLOB LUALIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/lualib/*.c")
    else()
      message(FATAL_ERROR "Cannot find Lua library source files")
    endif()
  endif()
else()
  # Try alternative structure
  file(GLOB LUA_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib")
    file(GLOB LUALIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lib/*.c")
  else()
    message(FATAL_ERROR "Cannot find Lua source files")
  endif()
endif()

# Print found sources
message(STATUS "Lua sources: ${LUA_SOURCES}")
message(STATUS "Lualib sources: ${LUALIB_SOURCES}")

# Exclude luac.c from the sources as it's a separate executable
list(FILTER LUA_SOURCES EXCLUDE REGEX ".*luac.c$")

# If no sources were found, use hardcoded list as fallback
if(NOT LUA_SOURCES)
  message(STATUS "Using hardcoded Lua source list")
  set(LUA_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lcode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldebug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ldump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lgc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/llex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lobject.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lparser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lstate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lstring.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ltable.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ltests.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ltm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lundump.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lvm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lzio.c
  )
endif()

if(NOT LUALIB_SOURCES)
  message(STATUS "Using hardcoded Lualib source list")
  set(LUALIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/lauxlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/lbaselib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/ldblib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/liolib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/lmathlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/lstrlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/ltablib.c
  )
endif()

# Create the libraries
add_library(lua5 STATIC ${LUA_SOURCES})
add_library(lualib5 STATIC ${LUALIB_SOURCES})

# Include directories
target_include_directories(lua5 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(lualib5 PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Install Lua headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include/lua5
  FILES_MATCHING PATTERN "*.h"
)

# Add necessary compile definitions
target_compile_definitions(lua5 PRIVATE LUA_ANSI)
target_compile_definitions(lualib5 PRIVATE LUA_ANSI)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(lua5 PRIVATE ANDROID)
  target_compile_definitions(lualib5 PRIVATE ANDROID)
endif()

# On Unix-like systems, we need to link with math library
if(UNIX)
  target_link_libraries(lua5 PRIVATE m)
  target_link_libraries(lualib5 PRIVATE m)
endif()

# Export targets for other projects to use
install(TARGETS lua5 lualib5
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
)

# Create imported targets for other projects
export(TARGETS lua5 lualib5 FILE ${CMAKE_BINARY_DIR}/lua5-targets.cmake)
