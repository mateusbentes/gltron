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
  ${PNG_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
)

# Link with PNG and SDL2
target_link_libraries(nebu_video PRIVATE
  ${PNG_LIBRARIES}
  ${SDL2_LIBRARIES}
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_video PRIVATE ANDROID)
endif()
