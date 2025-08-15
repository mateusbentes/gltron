# Build all subdirectories first
message(STATUS "=== Building GLTron Modules ===")

add_subdirectory(base)
message(STATUS "✓ Added base module")

add_subdirectory(filesystem)
message(STATUS "✓ Added filesystem module")

add_subdirectory(configuration)
message(STATUS "✓ Added configuration module")

add_subdirectory(video)
message(STATUS "✓ Added video module")

add_subdirectory(audio)
message(STATUS "✓ Added audio module")

add_subdirectory(input)
message(STATUS "✓ Added input module")

# Check if scripting directory exists and add it BEFORE game module
set(SCRIPTING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripting")
if(EXISTS "${SCRIPTING_DIR}" AND EXISTS "${SCRIPTING_DIR}/CMakeLists.txt")
    add_subdirectory(scripting)
    set(SCRIPTING_AVAILABLE TRUE)
    message(STATUS "✓ Added scripting module")
else()
    set(SCRIPTING_AVAILABLE FALSE)
    message(STATUS "⚠ Scripting module not available (directory: ${SCRIPTING_DIR})")
endif()

# Build game module last since it depends on others
add_subdirectory(game)
message(STATUS "✓ Added game module")

# Create the main executable
message(STATUS "=== Creating GLTron Executable ===")

# Resolve and verify main source file
set(GLTRON_MAIN "${CMAKE_CURRENT_SOURCE_DIR}/gltron.c")
if(NOT EXISTS "${GLTRON_MAIN}")
    message(FATAL_ERROR "Expected main source not found: ${GLTRON_MAIN}")
endif()
message(STATUS "Using main source for gltron: ${GLTRON_MAIN}")

# Add the executable
if(BUILD_ANDROID)
    # For Android, we need to create a shared library that will be loaded by Java
    add_library(gltron SHARED "${CMAKE_CURRENT_SOURCE_DIR}/gltron.c")
    message(STATUS "Created Android shared library for gltron")
else()
    # For desktop, create a regular executable
    add_executable(gltron "${CMAKE_CURRENT_SOURCE_DIR}/gltron.c")
    message(STATUS "Created desktop executable for gltron")
endif()

# Add includes and definitions
add_common_includes(gltron)
add_common_platform_definitions(gltron)

# Link libraries in dependency order
message(STATUS "Linking libraries to gltron executable...")

target_link_libraries(gltron PRIVATE
    # Main game library first (it uses functions from other modules)
    game

    # Then the modules that game depends on
    video
    audio
    input
    configuration
    filesystem
    base

    # Nebu engine libraries (in dependency order)
    nebu_video
    nebu_audio
    nebu_input
    nebu_filesystem
    nebu_system
    nebu_base

    # External libraries
    lua5
    #glew
)

# FIXED: Only add scripting libraries if they actually exist
if(SCRIPTING_AVAILABLE AND TARGET scripting AND TARGET nebu_scripting)
    target_link_libraries(gltron PRIVATE
        scripting
        nebu_scripting
        lualib5
    )
    message(STATUS "✓ Linked scripting libraries")
else()
    message(STATUS "⚠ Scripting libraries not available - building without scripting support")
endif()

# If XMP is enabled
if(ENABLE_XMP)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBXMP REQUIRED libxmp)

    if (LIBXMP_FOUND)
        message(STATUS "✓ Found libxmp: ${LIBXMP_LIBRARIES}")
        include_directories(${LIBXMP_INCLUDE_DIRS})
        link_directories(${LIBXMP_LIBRARY_DIRS})
        target_link_libraries(gltron PRIVATE ${LIBXMP_LIBRARIES})
    else()
        message(WARNING "✗ libxmp not found - IT music support disabled")
        set(ENABLE_XMP OFF)
    endif()
endif()

# Platform-specific linking
if(BUILD_ANDROID)
    target_compile_definitions(gltron PRIVATE SDL_MAIN_HANDLED GLTRON_BUILD_ANDROID=1)
    target_link_libraries(gltron PRIVATE
        ${ANDROID_LIB}
        ${LOG_LIB}
        ${EGL_LIB}
        ${GLES_LIB}
    )

    if(ENABLE_SOUND AND OPENSLES_LIB)
        target_link_libraries(gltron PRIVATE ${OPENSLES_LIB})
    endif()

    # Set proper output name for Android
    set_target_properties(gltron PROPERTIES
        OUTPUT_NAME "libgltron"
        PREFIX ""
        SUFFIX ".so"
    )

    message(STATUS "✓ Added Android-specific libraries")
else()
    # Ensure target-level platform macro to avoid ambiguity
    target_compile_definitions(gltron PRIVATE SDL_MAIN_HANDLED GLTRON_BUILD_ANDROID=0 ANDROID=0)

    # Linux/Desktop linking
    add_common_sdl_linking(gltron)

    target_link_libraries(gltron PRIVATE
        ${ZLIB_LIBRARIES}
        ${PNG_LIBRARIES}
        #GLEW::GLEW
        ${OPENGL_LIBRARIES}
    )

    if(ENABLE_SOUND)
        target_link_libraries(gltron PRIVATE ${SDL2_MIXER_LIBRARIES})
    endif()

    if(ENABLE_TTF)
        target_link_libraries(gltron PRIVATE ${SDL2_TTF_LIBRARIES})
    endif()

    message(STATUS "✓ Added Linux/Desktop libraries")
endif()

# Set output properties
set_target_properties(gltron PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Debug: Print final linking information
get_target_property(GLTRON_LINK_LIBS gltron LINK_LIBRARIES)
message(STATUS "=== Final GLTron Link Libraries ===")
if(GLTRON_LINK_LIBS)
    foreach(lib ${GLTRON_LINK_LIBS})
        message(STATUS "  → ${lib}")
    endforeach()
else()
    message(STATUS "  → No link libraries found")
endif()

get_target_property(GLTRON_SOURCES gltron SOURCES)
message(STATUS "gltron sources: ${GLTRON_SOURCES}") 

# Debug: Check what targets exist
if(TARGET game)
    message(STATUS "✓ Game library target exists")
else()
    message(WARNING "✗ Game library target does not exist!")
endif()

if(TARGET scripting)
    message(STATUS "✓ Scripting library target exists")
else()
    message(STATUS "⚠ Scripting library target does not exist")
endif()

if(TARGET nebu_scripting)
    message(STATUS "✓ Nebu scripting library target exists")
else()
    message(STATUS "⚠ Nebu scripting library target does not exist")
endif()

message(STATUS "=== End GLTron Build Configuration ===")
