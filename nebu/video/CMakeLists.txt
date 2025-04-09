# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Explicitly include vector.c
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/vector.c")

# Create the library
add_library(nebu_video STATIC ${SOURCES} ${HEADERS})

# Find required packages
find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)

# Include directories
target_include_directories(nebu_video PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/src/include
  ${PNG_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
)

# Link with sdl_compat first, then PNG and SDL2 libraries
target_link_libraries(nebu_video PUBLIC
  sdl_compat
  ${PNG_LIBRARIES}
  ${SDL2_LIBRARIES}
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_video PRIVATE ANDROID)
endif()
