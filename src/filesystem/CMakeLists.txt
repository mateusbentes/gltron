
# Find all common source files in this directory
file(GLOB COMMON_SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Remove all platform-specific files from common sources
list(FILTER COMMON_SOURCES EXCLUDE REGEX "dirsetup-(macos|win32)\\.c$")

# Set platform-specific source files
set(PLATFORM_SOURCES "")

if(MACOSX)
    # Use macOS-specific implementation
    list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dirsetup-macos.c")
elseif(WIN32)
    # Use Windows-specific implementation
    list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dirsetup-win32.c")
else()
    # Use the generic implementation for Linux/Unix
    # No additional files needed as dirsetup.c is already in COMMON_SOURCES
endif()

# Create the library
add_library(filesystem STATIC ${COMMON_SOURCES} ${PLATFORM_SOURCES} ${HEADERS})

# Include directories
target_include_directories(filesystem PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
)

# Platform-specific configurations
if(MACOSX)
  target_compile_definitions(filesystem PRIVATE MACOSX)
elseif(WIN32)
  target_compile_definitions(filesystem PRIVATE WIN32)
elseif(BUILD_ANDROID)
  target_compile_definitions(filesystem PRIVATE ANDROID)
else()
  target_compile_definitions(filesystem PRIVATE LINUX)
endif()
