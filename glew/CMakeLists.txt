# Find all source files in this directory
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "include/GL/*.h")

# Create the library
add_library(glew STATIC ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(glew 
  PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Define GLEW preprocessor macros
target_compile_definitions(glew 
  PRIVATE 
    GLEW_STATIC
    GLEW_NO_GLU
)

# Platform-specific configurations
if(WIN32)
  target_compile_definitions(glew PRIVATE WIN32_LEAN_AND_MEAN)
  target_link_libraries(glew PRIVATE opengl32)
elseif(APPLE)
  target_compile_definitions(glew PRIVATE MACOSX)
  find_library(OPENGL_FRAMEWORK OpenGL)
  target_link_libraries(glew PRIVATE ${OPENGL_FRAMEWORK})
elseif(BUILD_ANDROID)
  target_compile_definitions(glew PRIVATE ANDROID)
else()
  # Linux
  target_link_libraries(glew PRIVATE GL)
endif()

# Export GLEW include directories to parent scope
set(GLEW_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)
