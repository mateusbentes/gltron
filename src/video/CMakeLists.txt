# Find all source files in this directory
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Create the library
add_library(video STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(video PUBLIC
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/nebu/include
  ${GLEW_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(video PUBLIC
  nebu_filesystem
  nebu_scripting  # Added for gui_ReleaseResources
  nebu_video
  nebu_base
  nebu_input     # Adding input library
  nebu_audio     # Adding audio library
  sdl_compat
  ${GLEW_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${SDL2_IMAGE_LIBRARIES}  # Adding SDL2_image
  ${OPENGL_LIBRARIES}
  ${CMAKE_DL_LIBS}         # Adding dynamic loading library
)

# Platform-specific configurations
if(BUILD_ANDROID)
  target_compile_definitions(video PRIVATE ANDROID)
endif()
