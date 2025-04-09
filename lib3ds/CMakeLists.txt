# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(lib3ds STATIC ${SOURCES} ${HEADERS})

# Include directories - the parent directory is needed for <lib3ds/...> style includes
target_include_directories(lib3ds PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..  # Add parent directory for <lib3ds/...> includes
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(lib3ds PRIVATE ANDROID)
endif()

# Export lib3ds include directory to parent scope
set(LIB3DS_INCLUDE_DIRS 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/..  # Add parent directory for <lib3ds/...> includes
  PARENT_SCOPE
)
