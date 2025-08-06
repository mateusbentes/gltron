# nebu/base/CMakeLists.txt
create_standard_library(nebu_base)

# Link with PNG and ZLIB libraries specific to nebu_base
target_link_libraries(nebu_base PUBLIC
  ${PNG_LIBRARIES}
  ${ZLIB_LIBRARIES}
)

add_common_sdl_linking(nebu_base)