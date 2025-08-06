# Add subdirectories for all components
add_subdirectory(configuration) 
add_subdirectory(base)
add_subdirectory(filesystem)
add_subdirectory(game)
add_subdirectory(input)
add_subdirectory(audio)
add_subdirectory(video)
add_subdirectory(scripting)

# Create the main executable
if(BUILD_ANDROID)
  # Android native library
  add_library(gltron_native SHARED gltron.c)
  set(TARGET_NAME gltron_native)
else()
  # Linux executable
  add_executable(gltron gltron.c)
  set(TARGET_NAME gltron)
endif()

# Add common includes and definitions
add_common_includes(${TARGET_NAME})
add_common_platform_definitions(${TARGET_NAME})

# Set target properties
set_target_properties(${TARGET_NAME} PROPERTIES
    C_STANDARD 99
    CXX_STANDARD 11
)

if(BUILD_ANDROID)
  set_target_properties(${TARGET_NAME} PROPERTIES
      POSITION_INDEPENDENT_CODE ON
  )
endif()

# Link all the module libraries
target_link_libraries(${TARGET_NAME} PRIVATE
    # Main modules (order matters for static linking)
    game
    input
    audio
    video
    configuration
    filesystem
    base
    src_scripting
    
    # Nebu libraries
    nebu_scripting
    nebu_video
    nebu_input
    nebu_filesystem
    nebu_base
    nebu_audio
    nebu_system
    
    # External libraries
    glew
    lua5
    lualib5
)

# Platform-specific linking
if(BUILD_ANDROID)
  # Android libraries
  target_link_libraries(${TARGET_NAME} PRIVATE
      ${ANDROID_LIB}
      ${LOG_LIB}
      ${EGL_LIB}
      ${GLES_LIB}
  )
  
  if(ENABLE_SOUND AND OPENSLES_LIB)
    target_link_libraries(${TARGET_NAME} PRIVATE ${OPENSLES_LIB})
  endif()
else()
  # Linux/Desktop libraries
  target_link_libraries(${TARGET_NAME} PRIVATE
      ${GLEW_LIBRARIES}
      ${PNG_LIBRARIES}
      ${ZLIB_LIBRARIES}
      ${OPENGL_LIBRARIES}
      ${SDL2_LIBRARIES}
  )
  
  if(ENABLE_SOUND)
    target_link_libraries(${TARGET_NAME} PRIVATE ${SDL2_MIXER_LIBRARIES})
  endif()
  
  # Dynamic loading (Unix only)
  if(NOT WIN32)
    target_link_libraries(${TARGET_NAME} PRIVATE dl)
  endif()
endif()

# Add math library
target_link_libraries(${TARGET_NAME} PRIVATE m)

# Installation rules
if(BUILD_ANDROID)
  install(TARGETS ${TARGET_NAME} 
      LIBRARY DESTINATION lib
      ARCHIVE DESTINATION lib
  )
else()
  install(TARGETS ${TARGET_NAME} DESTINATION bin)
endif()