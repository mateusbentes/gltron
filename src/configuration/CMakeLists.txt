# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")

# Create the library
add_library(configuration STATIC ${SOURCES})

# Include directories
target_include_directories(configuration PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(configuration PRIVATE ANDROID)
endif()

# Enable touch support on all platforms
target_compile_definitions(configuration PRIVATE ENABLE_TOUCH_SUPPORT)
