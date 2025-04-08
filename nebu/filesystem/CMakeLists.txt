# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(nebu_filesystem STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(nebu_filesystem PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_filesystem PRIVATE ANDROID)
endif()