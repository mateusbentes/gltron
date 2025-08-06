# src/input/CMakeLists.txt
create_standard_library(input)

# Link libraries specific to input
target_link_libraries(input PUBLIC
  nebu_scripting
  nebu_input
  nebu_base
  nebu_video
  game
)