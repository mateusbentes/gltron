# Initialize source files list
set(SOURCES "")
set(HEADERS "")

# Add common source files
file(GLOB COMMON_SOURCES "file_io.c" "path.c" "dirsetup.c" "filesystem.c")
file(GLOB COMMON_HEADERS "*.h")

list(APPEND SOURCES ${COMMON_SOURCES})
list(APPEND HEADERS ${COMMON_HEADERS})

# Add platform-specific source files
if(MACOSX)
  file(GLOB PLATFORM_SOURCES "directory-macos.c")
  list(APPEND SOURCES ${PLATFORM_SOURCES})
  set(PLATFORM_DEFINE MACOSX)
elseif(WIN32)
  file(GLOB PLATFORM_SOURCES "directory-win32.c")
  list(APPEND SOURCES ${PLATFORM_SOURCES})
  set(PLATFORM_DEFINE WIN32)
elseif(BUILD_ANDROID)
  file(GLOB PLATFORM_SOURCES "directory.c")
  list(APPEND SOURCES ${PLATFORM_SOURCES})
  set(PLATFORM_DEFINE ANDROID)
else()
  file(GLOB PLATFORM_SOURCES "directory.c")
  list(APPEND SOURCES ${PLATFORM_SOURCES})
  set(PLATFORM_DEFINE LINUX)
endif()

# Create the library
add_library(nebu_filesystem STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(nebu_filesystem PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/src/include
  ${SDL2_INCLUDE_DIRS}
)

# Link with SDL and sdl_compat
target_link_libraries(nebu_filesystem PUBLIC
  sdl_compat
  ${SDL2_LIBRARIES}
)

# Platform-specific configurations
target_compile_definitions(nebu_filesystem PRIVATE ${PLATFORM_DEFINE})

# Print information about which files are being compiled
message(STATUS "Filesystem sources: ${SOURCES}")
message(STATUS "Platform: ${PLATFORM_DEFINE}")
