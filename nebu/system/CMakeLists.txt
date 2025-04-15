# nebu/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Add source files
set(NEBU_SYSTEM_SOURCES
  nebu_system.c
  nebu_system_callbacks.c
  nebu_system_swap_buffers.c
)

# Create library
add_library(nebu_system STATIC ${NEBU_SYSTEM_SOURCES})

# Include directories
target_include_directories(nebu_system PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${SDL2_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(nebu_system PUBLIC
  ${SDL2_LIBRARIES}
  nebu_video
)

# Install library
install(TARGETS nebu_system DESTINATION lib)