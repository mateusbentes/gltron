# nebu/video/CMakeLists.txt
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Explicitly include vector.c
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/vector.c")

add_library(nebu_video STATIC ${SOURCES} ${HEADERS})
add_common_includes(nebu_video)
add_common_platform_definitions(nebu_video)

# Link with libraries specific to nebu_video
target_link_libraries(nebu_video PUBLIC
  nebu_filesystem
  nebu_base
  #GLEW::GLEW
  ${PNG_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

add_common_sdl_linking(nebu_video)