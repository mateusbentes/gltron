# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(nebu_base STATIC ${SOURCES} ${HEADERS})

# Find required packages
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SDL2 REQUIRED)

# Include directories
target_include_directories(nebu_base PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${CMAKE_SOURCE_DIR}/src/include
  ${PNG_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
)

# Link with libraries
target_link_libraries(nebu_base PUBLIC
  sdl_compat
  ${PNG_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${SDL2_LIBRARIES}
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_base PRIVATE ANDROID)
endif()
