# Configure a header file to pass some CMake settings to the source code
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gltron-config.h.in"
  "${CMAKE_BINARY_DIR}/include/gltron-config.h"
)

# Add the binary dir to the include path for the config header
include_directories("${CMAKE_BINARY_DIR}/include")