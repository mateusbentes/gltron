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

# Check for required sound files with game_ prefix
set(REQUIRED_PREFIXED_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/game_engine.wav"
    "${CMAKE_CURRENT_SOURCE_DIR}/game_crash.wav"
)

foreach(PREFIXED_FILE ${REQUIRED_PREFIXED_FILES})
    if(EXISTS ${PREFIXED_FILE})
        message(STATUS "Required sound file found: ${PREFIXED_FILE}")
    else()
        get_filename_component(FILENAME ${PREFIXED_FILE} NAME)
        message(WARNING "Missing required sound file: ${FILENAME}")
    endif()
endforeach()

# Copy all sound files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${CMAKE_BINARY_DIR}/sounds)
message(STATUS "Copied sound files to build directory")
