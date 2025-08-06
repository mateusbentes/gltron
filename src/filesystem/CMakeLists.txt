# src/filesystem/CMakeLists.txt
file(GLOB COMMON_SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Remove all platform-specific files from common sources
list(FILTER COMMON_SOURCES EXCLUDE REGEX "dirsetup-(macos|win32)\\.c$")

# Explicitly add installpath.c if it's not already included
list(APPEND COMMON_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/installpath.c")

# Set platform-specific source files
set(PLATFORM_SOURCES "")

if(MACOSX)
    list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dirsetup-macos.c")
elseif(WIN32)
    list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dirsetup-win32.c")
endif()

add_library(filesystem STATIC ${COMMON_SOURCES} ${PLATFORM_SOURCES} ${HEADERS})
add_common_includes(filesystem)
add_common_platform_definitions(filesystem)

# Link with nebu_filesystem
target_link_libraries(filesystem PUBLIC
  nebu_filesystem
)

add_common_sdl_linking(filesystem)