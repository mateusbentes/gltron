# Collect all source files
file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

# Debug: Print what we found initially
message(STATUS "=== Game Module Debug ===")
message(STATUS "Current directory: ${CMAKE_CURRENT_SOURCE_DIR}")

# Get the length of SOURCES list properly
list(LENGTH SOURCES SOURCES_COUNT)
message(STATUS "Initially found ${SOURCES_COUNT} source files:")

foreach(source ${SOURCES})
    get_filename_component(filename ${source} NAME)
    message(STATUS "  - ${filename}")
endforeach()

# Remove duplicate and problematic source files
# These files have duplicate function definitions with other files
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/scripting_helpers.c")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/level.c")  # Duplicate of game_level.c
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/c_setcallback.c")  # Duplicate of scripting_interface.c

# CRITICAL FIX: Also remove scripting-dependent files if scripting is not available
# Check if scripting directory exists in the parent directory
set(SCRIPTING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../scripting")
if(NOT EXISTS "${SCRIPTING_DIR}")
    message(STATUS "⚠ Scripting directory not found, removing scripting-dependent files")
    
    # Remove files that depend on scripting functions
    list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/scripting_interface.c")
    
    # You might need to remove other scripting-dependent files too
    # Uncomment these if they cause scripting-related linker errors:
    # list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/level.c")
    # list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/game_level.c")
    
    set(SCRIPTING_AVAILABLE FALSE)
    message(STATUS "✓ Scripting-dependent files removed from build")
else()
    set(SCRIPTING_AVAILABLE TRUE)
    message(STATUS "✓ Scripting directory found, keeping scripting files")
endif()

# Debug: Print sources after removal
message(STATUS "Sources after removing duplicates and scripting files:")
foreach(source ${SOURCES})
    get_filename_component(filename ${source} NAME)
    message(STATUS "  ✓ ${filename}")
endforeach()

# Verify menu.c is included
set(MENU_C_PATH "${CMAKE_CURRENT_SOURCE_DIR}/menu.c")
if(EXISTS "${MENU_C_PATH}")
    message(STATUS "✓ menu.c confirmed at: ${MENU_C_PATH}")
else()
    message(FATAL_ERROR "✗ menu.c missing at: ${MENU_C_PATH}")
endif()

# Verify game_level.c exists (should be used instead of level.c)
set(GAME_LEVEL_C_PATH "${CMAKE_CURRENT_SOURCE_DIR}/game_level.c")
if(EXISTS "${GAME_LEVEL_C_PATH}")
    message(STATUS "✓ game_level.c confirmed (level.c excluded to avoid duplicates)")
else()
    message(WARNING "⚠ game_level.c not found - level functions may be missing")
endif()

# Create the static library
add_library(game STATIC ${SOURCES} ${HEADERS})

# Add includes and definitions
add_common_includes(game)
add_common_platform_definitions(game)

# Link dependencies - game module needs these other modules
target_link_libraries(game PUBLIC
    # Core dependencies that provide missing functions
    configuration    # Provides settings_*, getSettingi, getSettingf, etc.
    filesystem      # Provides getDirectory, getPossiblePath, getPath, etc.
    input          # Provides Input_Idle
    video          # Provides doPngScreenShot, doBmpScreenShot
    base           # Provides basic utilities
    
    # Nebu engine libraries
    nebu_filesystem
    nebu_input
    nebu_video
    nebu_base
)

# FIXED: Only link scripting libraries if they actually exist and are built
if(SCRIPTING_AVAILABLE AND TARGET scripting AND TARGET nebu_scripting)
    target_link_libraries(game PUBLIC
        scripting       # The scripting module
        nebu_scripting  # Nebu scripting engine
        lua5           # Lua interpreter
        lualib5        # Lua standard libraries
    )
    message(STATUS "✓ Game module linked with scripting support")
    
    # Add scripting compile definition
    target_compile_definitions(game PRIVATE SCRIPTING_ENABLED=1)
else()
    message(STATUS "⚠ Game module built WITHOUT scripting support")
    
    # Add no-scripting compile definition
    target_compile_definitions(game PRIVATE SCRIPTING_ENABLED=0)
endif()

# Add SDL linking
add_common_sdl_linking(game)

# Set library properties
set_target_properties(game PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

message(STATUS "=== End Game Module Debug ===")
