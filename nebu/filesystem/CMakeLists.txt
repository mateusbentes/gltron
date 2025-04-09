# Find all common source files in this directory
file(GLOB COMMON_SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Remove all platform-specific files from common sources
list(FILTER COMMON_SOURCES EXCLUDE REGEX "(filesystem|directory|file)-(macos|win32|linux|android)\\.c$")

# Set platform-specific source files
set(PLATFORM_SOURCES "")

# Check for platform-specific filesystem implementation
if(MACOSX)
    # Use macOS-specific implementations
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-macos.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-macos.c")
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/directory-macos.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/directory-macos.c")
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/file-macos.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/file-macos.c")
    endif()
elseif(WIN32)
    # Use Windows-specific implementations
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-win32.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-win32.c")
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/directory-win32.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/directory-win32.c")
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/file-win32.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/file-win32.c")
    endif()
else()
    # Use Linux/Android implementations
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-android.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-android.c")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-linux.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-linux.c")
    endif()
    
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/file-linux.c")
        list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/file-linux.c")
    endif()
endif()

# If we're building for Android but don't have filesystem-android.c, create a stub
if(BUILD_ANDROID AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/filesystem-android.c" AND NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/filesystem-android-stub.c")
    # Create a stub implementation for Android
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/filesystem-android-stub.c" 
"/* Auto-generated stub implementation for Android */
#include \"Nebu_filesystem.h\"
#include <stdio.h>

int nebu_FS_Test(const char *path) {
    FILE *f = fopen(path, \"r\");
    if(f) {
        fclose(f);
        return 1;
    }
    return 0;
}

void nebu_FS_Delete(const char *path) {
    remove(path);
}
")
    list(APPEND PLATFORM_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/filesystem-android-stub.c")
endif()

# Create the library
add_library(nebu_filesystem STATIC ${COMMON_SOURCES} ${PLATFORM_SOURCES} ${HEADERS})

# Include directories
target_include_directories(nebu_filesystem PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
)

# Platform-specific configurations
if(MACOSX)
  target_compile_definitions(nebu_filesystem PRIVATE MACOSX)
elseif(WIN32)
  target_compile_definitions(nebu_filesystem PRIVATE WIN32)
elseif(BUILD_ANDROID)
  target_compile_definitions(nebu_filesystem PRIVATE ANDROID)
else()
  target_compile_definitions(nebu_filesystem PRIVATE LINUX)
endif()
