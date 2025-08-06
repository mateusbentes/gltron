# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(lib3ds STATIC ${SOURCES} ${HEADERS})

# Include directories - the parent directory is needed for <lib3ds/...> style includes
target_include_directories(lib3ds PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>  # Add parent directory for <lib3ds/...> includes
  $<INSTALL_INTERFACE:include>
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(lib3ds PRIVATE ANDROID)
endif()

# Export lib3ds include directory to parent scope
set(LIB3DS_INCLUDE_DIRS
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>  # Add parent directory for <lib3ds/...> includes
  PARENT_SCOPE
)

# Install the library and headers
install(TARGETS lib3ds
  EXPORT lib3dsTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION include
)

# Export the targets
install(EXPORT lib3dsTargets
  FILE lib3dsTargets.cmake
  NAMESPACE lib3ds::
  DESTINATION lib/cmake/lib3ds
)
