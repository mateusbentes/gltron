# Print a message during configuration to verify this is being processed
message(STATUS "Processing sounds directory")

# List all sound files in the directory
file(GLOB SOUND_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.wav")

# Display all found sound files
message(STATUS "Found sound files:")
foreach(SOUND_FILE ${SOUND_FILES})
    get_filename_component(FILENAME ${SOUND_FILE} NAME)
    message(STATUS "  - ${FILENAME}")
endforeach()

# Check for required sound files
set(REQUIRED_SOUND_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/engine.wav"
    "${CMAKE_CURRENT_SOURCE_DIR}/crash.wav"
)

# Check for optional recognizer sound
set(RECOGNIZER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/recognizer.wav")
if(EXISTS ${RECOGNIZER_FILE})
    message(STATUS "Recognizer sound file found: ${RECOGNIZER_FILE}")
else()
    message(WARNING "Missing recognizer sound file. Some game sounds may not work.")
    
    # Try to create a recognizer.wav from another sound file
    message(STATUS "Attempting to create a substitute recognizer.wav file...")
    
    # Try to use game_win.wav or another suitable file as a substitute
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/game_win.wav")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/game_win.wav" ${RECOGNIZER_FILE} COPYONLY)
        message(STATUS "Created substitute recognizer.wav from game_win.wav")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/game_start.wav")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/game_start.wav" ${RECOGNIZER_FILE} COPYONLY)
        message(STATUS "Created substitute recognizer.wav from game_start.wav")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/engine.wav")
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/engine.wav" ${RECOGNIZER_FILE} COPYONLY)
        message(STATUS "Created substitute recognizer.wav from engine.wav")
    else()
        message(WARNING "Could not create a substitute recognizer.wav file")
    endif()
endif()

foreach(SOUND_FILE ${REQUIRED_SOUND_FILES})
    if(EXISTS ${SOUND_FILE})
        message(STATUS "Required sound file found: ${SOUND_FILE}")
    else()
        message(WARNING "Missing required sound file: ${SOUND_FILE}")
        
        # Check if there's a similar file with "game_" prefix
        get_filename_component(FILENAME ${SOUND_FILE} NAME)
        set(GAME_FILE "${CMAKE_CURRENT_SOURCE_DIR}/game_${FILENAME}")
        if(EXISTS ${GAME_FILE})
            message(STATUS "  But found alternative: game_${FILENAME}")
            # Create a symlink or copy the file
            if(WIN32)
                configure_file(${GAME_FILE} ${SOUND_FILE} COPYONLY)
                message(STATUS "  Copied ${GAME_FILE} to ${SOUND_FILE}")
            else()
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E create_symlink ${GAME_FILE} ${SOUND_FILE}
                    RESULT_VARIABLE LINK_RESULT
                )
                if(LINK_RESULT EQUAL 0)
                    message(STATUS "  Created symlink from ${GAME_FILE} to ${SOUND_FILE}")
                else()
                    configure_file(${GAME_FILE} ${SOUND_FILE} COPYONLY)
                    message(STATUS "  Copied ${GAME_FILE} to ${SOUND_FILE}")
                endif()
            endif()
        endif()
    endif()
endforeach()


# Copy all sound files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${CMAKE_BINARY_DIR}/sounds)
message(STATUS "Copied sound files to build directory")
