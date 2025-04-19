# Collect all C source files in this directory
file(GLOB CONFIGURATION_SOURCES CONFIGURATION_HEADERS
  "*.c"
  "*.h"
)

# Create a static library from the source files
add_library(configuration STATIC ${CONFIGURATION_SOURCES})

# Set include directories for this library
target_include_directories(configuration PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include        # Project-wide headers
  ${CMAKE_SOURCE_DIR}/nebu/include       # Nebu engine headers
  ${CMAKE_CURRENT_SOURCE_DIR}            # Local headers (e.g., settings.h)
)

# Define platform-specific macros
if(BUILD_ANDROID)
  target_compile_definitions(configuration PRIVATE ANDROID)
endif()

# Always enable touch support
target_compile_definitions(configuration PRIVATE ENABLE_TOUCH_SUPPORT)
