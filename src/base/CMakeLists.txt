# src/base/CMakeLists.txt
file(GLOB SOURCES "*.c" "*.cpp")
file(GLOB HEADERS "*.h")

# Make sure sdl_compat.c and setcallback.c are included
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/sdl_compat.c")
list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/setcallback.c")

add_library(base STATIC ${SOURCES} ${HEADERS})
add_common_includes(base)
add_common_platform_definitions(base)

add_common_sdl_linking(base)