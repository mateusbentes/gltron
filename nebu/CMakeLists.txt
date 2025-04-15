# nebu/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# Add source files for nebu_system
set(NEBU_SYSTEM_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/system/nebu_system_callbacks.c
  ${CMAKE_CURRENT_SOURCE_DIR}/system/nebu_system_swap_buffers.c
)

# Create nebu_system library
add_library(nebu_system STATIC ${NEBU_SYSTEM_SOURCES})

# Include directories for nebu_system
target_include_directories(nebu_system PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${CMAKE_CURRENT_SOURCE_DIR}/../include
  ${SDL2_INCLUDE_DIRS}
)

# Link libraries for nebu_system
target_link_libraries(nebu_system PUBLIC
  ${SDL2_LIBRARIES}
  nebu_video
)

# Install nebu_system library
install(TARGETS nebu_system DESTINATION lib)

# Add subdirectories for other nebu components
add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(video)
add_subdirectory(scripting)
add_subdirectory(filesystem)
add_subdirectory(base)
