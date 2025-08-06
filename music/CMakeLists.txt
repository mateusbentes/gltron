# Print a message during configuration to verify this is being processed
message(STATUS "Processing music directory")

# Check for music files
file(GLOB MUSIC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.it" "${CMAKE_CURRENT_SOURCE_DIR}/*.ogg" "${CMAKE_CURRENT_SOURCE_DIR}/*.mp3")
if(MUSIC_FILES)
    message(STATUS "Found music files:")
    foreach(MUSIC_FILE ${MUSIC_FILES})
        get_filename_component(FILENAME ${MUSIC_FILE} NAME)
        message(STATUS "  - ${FILENAME}")
    endforeach()
else()
    message(WARNING "No music files found in music directory")
    
    # Check if there are music files in the sounds directory
    file(GLOB SOUND_DIR_MUSIC_FILES "${CMAKE_SOURCE_DIR}/sounds/*.it" "${CMAKE_SOURCE_DIR}/sounds/*.ogg" "${CMAKE_SOURCE_DIR}/sounds/*.mp3")
    if(SOUND_DIR_MUSIC_FILES)
        message(STATUS "Found music files in sounds directory:")
        foreach(MUSIC_FILE ${SOUND_DIR_MUSIC_FILES})
            get_filename_component(FILENAME ${MUSIC_FILE} NAME)
            message(STATUS "  - ${FILENAME}")
            
            # Copy music files from sounds directory to music directory
            configure_file(${MUSIC_FILE} "${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME}" COPYONLY)
            message(STATUS "  Copied ${FILENAME} to music directory")
        endforeach()
    else()
        message(WARNING "No music files found in sounds directory either")
    endif()
endif()

# Copy all music files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${CMAKE_BINARY_DIR}/music
     FILES_MATCHING PATTERN "*.it" PATTERN "*.ogg" PATTERN "*.mp3")
message(STATUS "Copied music files to build directory")
