# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(input STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(input PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
)

# Link libraries
target_link_libraries(input PUBLIC
  nebu_scripting
  nebu_input
  nebu_base
  nebu_video
  game
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(input PRIVATE ANDROID)
endif()
