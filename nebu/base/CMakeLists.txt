# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(nebu_base STATIC ${SOURCES} ${HEADERS})

# Find required packages
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

# Include directories
target_include_directories(nebu_base PUBLIC
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${PNG_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
)

# Link with PNG and ZLIB
target_link_libraries(nebu_base PRIVATE
  ${PNG_LIBRARIES}
  ${ZLIB_LIBRARIES}
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(nebu_base PRIVATE ANDROID)
endif()
