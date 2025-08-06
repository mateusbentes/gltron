# src/video/CMakeLists.txt
create_standard_library(video)

# Link libraries specific to video
target_link_libraries(video PUBLIC
  nebu_filesystem
  nebu_scripting
  nebu_video
  nebu_base
  nebu_input
  nebu_audio
  ${GLEW_LIBRARIES}
  ${SDL2_IMAGE_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${CMAKE_DL_LIBS}
)

add_common_sdl_linking(video)