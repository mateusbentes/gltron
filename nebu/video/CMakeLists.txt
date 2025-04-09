# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Explicitly include vector.c
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/vector.c")

# Create the library
add_library(nebu_video STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(nebu_video PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_video PRIVATE ANDROID)
endif()
