# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Make sure sdl_compat.c and setcallback.c are included
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sdl_compat.c")
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/setcallback.c")

# Create the library
add_library(base STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(base PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(base PUBLIC
  ${SDL2_LIBRARIES}
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(base PRIVATE ANDROID)
endif()
