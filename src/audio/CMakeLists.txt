# src/audio/CMakeLists.txt
create_standard_library(audio)

# Link libraries specific to audio
target_link_libraries(audio PUBLIC
  nebu_filesystem
  nebu_audio
)

add_common_sdl_linking(audio)