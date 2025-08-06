# nebu/audio/CMakeLists.txt
create_standard_library(nebu_audio)

# Link libraries specific to nebu_audio
target_link_libraries(nebu_audio PUBLIC
  nebu_base
)

add_common_sdl_linking(nebu_audio)