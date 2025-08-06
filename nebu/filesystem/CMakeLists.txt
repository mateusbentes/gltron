# nebu/filesystem/CMakeLists.txt
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
elseif(WIN32)
  file(GLOB PLATFORM_SOURCES "directory-win32.c")
  list(APPEND SOURCES ${PLATFORM_SOURCES})
else()
  file(GLOB PLATFORM_SOURCES "directory.c")
  list(APPEND SOURCES ${PLATFORM_SOURCES})
endif()

add_library(nebu_filesystem STATIC ${SOURCES} ${HEADERS})
add_common_includes(nebu_filesystem)
add_common_platform_definitions(nebu_filesystem)

add_common_sdl_linking(nebu_filesystem)

# Print information about which files are being compiled
message(STATUS "Filesystem sources: ${SOURCES}")