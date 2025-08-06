# nebu/system/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)

# List possible source files
set(NEBU_SYSTEM_CANDIDATES
  nebu_system.c
  nebu_system_callbacks.c
  nebu_system_swap_buffers.c
)

# Check which files exist
set(NEBU_SYSTEM_SOURCES)
foreach(file ${NEBU_SYSTEM_CANDIDATES})
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    list(APPEND NEBU_SYSTEM_SOURCES ${file})
  else()
    message(STATUS "[nebu/system] Source file not found: ${file}")
  endif()
endforeach()

# Create library if sources exist, otherwise create dummy interface
if(NEBU_SYSTEM_SOURCES)
  add_library(nebu_system STATIC ${NEBU_SYSTEM_SOURCES})
  target_include_directories(nebu_system PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${SDL2_INCLUDE_DIRS}
  )
  target_link_libraries(nebu_system PUBLIC
    ${SDL2_LIBRARIES}
    nebu_video
  )
  install(TARGETS nebu_system DESTINATION lib)
  message(STATUS "[nebu/system] Created nebu_system library with sources: ${NEBU_SYSTEM_SOURCES}")
else()
  add_library(nebu_system INTERFACE)
  message(STATUS "[nebu/system] Created dummy nebu_system library (no source files found)")
endif()
