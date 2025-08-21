cmake_minimum_required(VERSION 3.10)
project(gltron)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Suppress OpenGL preference warning
cmake_policy(SET CMP0072 NEW)

# Informative output about platform selection
message(STATUS "Configuring gltron for platform: ${CMAKE_SYSTEM_NAME}")
if(ANDROID)
    message(STATUS "Android build detected: ANDROID macro will be defined; linking to log/android/EGL/GLESv2")
else()
    message(STATUS "Non-Android build: using desktop OpenGL and GLUT/GLU if available")
endif()

# Set OpenGL preference to legacy
set(OpenGL_GL_PREFERENCE "LEGACY")

# Enable sound by default
option(USE_SOUND "Enable sound support" ON)

# Add SOUND macro definition if USE_SOUND is enabled
if(USE_SOUND)
    add_definitions(-DSOUND)
endif()

# Try to find GLUT with multiple package names
if(NOT ANDROID)
    # Try common package names for GLUT
    find_package(GLUT QUIET)
    if(NOT GLUT_FOUND)
        find_package(glut QUIET)
        if(NOT glut_FOUND)
            find_package(freeglut QUIET)
            if(NOT freeglut_FOUND)
                message(WARNING "Could not find GLUT or freeglut. Game will run without GUI.")
                set(USE_GUI OFF)
            else()
                set(USE_GUI ON)
            endif()
        else()
            set(USE_GUI ON)
        endif()
    else()
        set(USE_GUI ON)
    endif()
else()
    set(USE_GUI ON)
endif()

# Find MikMod and related packages for sound support
if(USE_SOUND)
    # Ensure CMake can find the local FindMikMod.cmake module
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

    # Use find_package which will now consult the local module
    if(ANDROID)
        # For Android, use the Android-specific MikMod library
        find_package(MikMod REQUIRED)
    else()
        # For non-Android platforms, use the standard MikMod library
        find_package(MikMod REQUIRED)

        # Add support for WAV files by linking with additional libraries
        # Try multiple package names for Vorbis
        if(USE_WAV_SUPPORT)
            find_package(Vorbis QUIET)
            if(NOT Vorbis_FOUND)
                find_package(vorbis QUIET)
                if(NOT vorbis_FOUND)
                    find_package(libvorbis QUIET)
                    if(NOT libvorbis_FOUND)
                        message(WARNING "Could not find Vorbis. WAV file support will be limited.")
                        set(USE_WAV_SUPPORT OFF)
                    endif()
                endif()
            endif()
        endif()

        # Try multiple package names for FLAC
        if(USE_WAV_SUPPORT)
            find_package(FLAC QUIET)
            if(NOT FLAC_FOUND)
                find_package(flac QUIET)
                if(NOT flac_FOUND)
                    find_package(libflac QUIET)
                    if(NOT libflac_FOUND)
                        message(WARNING "Could not find FLAC. WAV file support will be limited.")
                        set(USE_WAV_SUPPORT OFF)
                    endif()
                endif()
            endif()
        endif()

        # Try multiple package names for Vorbis
        if(USE_WAV_SUPPORT)
            find_package(Vorbis QUIET)
            if(NOT Vorbis_FOUND)
                find_package(vorbis QUIET)
                if(NOT vorbis_FOUND)
                    find_package(libvorbis QUIET)
                    if(NOT libvorbis_FOUND)
                        message(WARNING "Could not find Vorbis. WAV file support will be limited.")
                        set(USE_WAV_SUPPORT OFF)
                    endif()
                endif()
            endif()
        endif()
    endif()
endif()

# Set source files (C)
set(SOURCES
    sgi_texture.c
    switchCallbacks.c
    gui.c
    gui_mouse.c
    game_mouse.c
    pause.c
    computer.c
    engine.c
    gltron.c
    graphics.c
    gamegraphics.c
    input.c
    settings.c
    texture.c
    fonttex.c
    fonts.c
    menu.c
    file.c
    model.c
    modelgraphics.c
    mtllib.c
    geom.c
    globals.c
)

# Add Android-only sources
if(ANDROID)
    list(APPEND SOURCES android_glue.c android_main.c)
endif()

# Ensure C sources are compiled as C (not C++)
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)
if(USE_SOUND)
    list(APPEND SOURCES sound.c)
endif()

# Add executable
add_executable(gltron ${SOURCES})

# Include directories
target_include_directories(gltron PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Link libraries
if(ANDROID)
    # Ensure ANDROID macro is defined for sources
    target_compile_definitions(gltron PRIVATE ANDROID)

    # Android-specific libraries (no desktop OpenGL)
    find_library(LOG_LIBRARY log)
    find_library(ANDROID_LIBRARY android)
    find_library(EGL_LIBRARY EGL)
    find_library(GLESv2_LIBRARY GLESv2)
    target_link_libraries(gltron PRIVATE
        ${LOG_LIBRARY}
        ${ANDROID_LIBRARY}
        ${EGL_LIBRARY}
        ${GLESv2_LIBRARY}
    )
else()
    find_package(OpenGL REQUIRED)
    # Non-Android libraries
    if(USE_GUI)
        target_link_libraries(gltron PRIVATE
            OpenGL::GL
            GLUT::GLUT
            OpenGL::GLU
        )
    else()
        target_link_libraries(gltron PRIVATE
            OpenGL::GL
            OpenGL::GLU
        )
    endif()
    target_link_libraries(gltron PRIVATE m)
endif()

if(USE_SOUND)
    if(ANDROID)
        # Android: use libopenmpt + OpenSL ES backend
        target_compile_definitions(gltron PRIVATE SOUND_BACKEND_OPENMPT)
        # Expect user to pass OpenMPT include/lib paths
        if(DEFINED OpenMPT_INCLUDE_DIRS)
            target_include_directories(gltron PRIVATE ${OpenMPT_INCLUDE_DIRS})
        endif()
        if(DEFINED OpenMPT_LIBRARIES)
            target_link_libraries(gltron PRIVATE ${OpenMPT_LIBRARIES})
        else()
            message(WARNING "OpenMPT_LIBRARIES not set. Please provide -DOpenMPT_LIBRARIES=/path/to/libopenmpt.a")
        endif()
        find_library(OPENSLES_LIB OpenSLES)
        if(OPENSLES_LIB)
            target_link_libraries(gltron PRIVATE ${OPENSLES_LIB})
        else()
            message(FATAL_ERROR "OpenSLES library not found")
        endif()
        # Add backend source
        target_sources(gltron PRIVATE backend_openmpt_android.c)
    else()
        # Desktop: use MikMod as before
        target_link_libraries(gltron PRIVATE ${MikMod_LIBRARIES})
        target_include_directories(gltron PRIVATE ${MikMod_INCLUDE_DIRS})
        target_sources(gltron PRIVATE backend_mikmod.c)

        # Link additional libraries for WAV support
        if(USE_WAV_SUPPORT)
            target_link_libraries(gltron PRIVATE
                ${Vorbis_LIBRARIES}
                ${FLAC_LIBRARIES}
                ${Ogg_LIBRARIES}
            )
        endif()
    endif()
endif()

# Set default install directories before install() commands
set(GLTRON_INSTALLDIR "/usr/bin" CACHE PATH "Installation directory for gltron executable")
set(GLTRON_HOME "/usr/share/games/gltron" CACHE PATH "Installation directory for gltron data files")

# List of files to copy
set(FILES_TO_COPY
    gltron.it
    settings.txt
    menu.txt
    tron.mtl
    t-u-low.obj
    xenotron.ftx
    xenotron.0.sgi
    xenotron.1.sgi
    gltron_floor.sgi
    gltron.sgi
    gltron_wall.sgi
    gltron_crash.sgi
)

# Add sound files only if sound is enabled
if(USE_SOUND)
    list(APPEND FILES_TO_COPY
        game_crash.wav
        game_lose.wav
        game_win.wav
        menu_highlight.wav
        game_engine.wav
        game_start.wav
        menu_action.wav
    )
endif()

# Copy files to the current directory
foreach(FILE ${FILES_TO_COPY})
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/${FILE}
        COPYONLY
    )
endforeach()

# Install targets
install(TARGETS gltron RUNTIME DESTINATION ${GLTRON_INSTALLDIR})

# Install files
install(FILES
    gltron.it
    settings.txt
    menu.txt
    tron.mtl
    t-u-low.obj
    xenotron.ftx
    xenotron.0.sgi
    xenotron.1.sgi
    gltron_floor.sgi
    gltron.sgi
    gltron_wall.sgi
    gltron_crash.sgi
    DESTINATION ${GLTRON_HOME}
)

# Install sound files only if sound is enabled
if(USE_SOUND)
    install(FILES
        game_crash.wav
        game_lose.wav
        game_win.wav
        menu_highlight.wav
        game_engine.wav
        game_start.wav
        menu_action.wav
        DESTINATION ${GLTRON_HOME}
    )
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    target_link_libraries(gltron PRIVATE opengl32 glu32)
    if(USE_SOUND)
        target_link_libraries(gltron PRIVATE winmm)
    endif()
elseif(APPLE)
    # macOS-specific settings
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(gltron PRIVATE ${COCOA_LIBRARY})
elseif(ANDROID)
    # Android-specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
    # Define ANDROID for C sources
    target_compile_definitions(gltron PRIVATE ANDROID)
    # Link Android libraries (no desktop OpenGL)
    find_library(LOG_LIBRARY log)
    find_library(ANDROID_LIBRARY android)
    find_library(EGL_LIBRARY EGL)
    find_library(GLESv2_LIBRARY GLESv2)
    target_link_libraries(gltron PRIVATE
        ${LOG_LIBRARY}
        ${ANDROID_LIBRARY}
        ${EGL_LIBRARY}
        ${GLESv2_LIBRARY}
    )
endif()
