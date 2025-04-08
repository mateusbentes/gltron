# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(lib3ds STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(lib3ds PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(lib3ds PRIVATE ANDROID)
endif()