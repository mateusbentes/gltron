cmake_minimum_required(VERSION 3.10)
project(gltron)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Suppress OpenGL preference warning
cmake_policy(SET CMP0072 NEW)

# Set OpenGL preference to legacy
set(OpenGL_GL_PREFERENCE "LEGACY")

# Enable sound by default
option(USE_SOUND "Enable sound support" ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

if(USE_SOUND)
    # Ensure CMake can find the local FindMikMod.cmake module
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
    # Use find_package which will now consult the local module
    find_package(MikMod REQUIRED)
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
)

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
target_link_libraries(gltron PRIVATE
    OpenGL::GL
    GLUT::GLUT
    OpenGL::GLU
    m
)

if(USE_SOUND)
    target_link_libraries(gltron PRIVATE ${MikMod_LIBRARIES})
    target_include_directories(gltron PRIVATE ${MikMod_INCLUDE_DIRS})
endif()

# Include directories
target_include_directories(gltron PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set default install directories before install() commands
set(GLTRON_INSTALLDIR "/usr/bin" CACHE PATH "Installation directory for gltron executable")
set(GLTRON_HOME "/usr/share/games/gltron" CACHE PATH "Installation directory for gltron data files")

# List of files to copy
set(FILES_TO_COPY
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
    # Link Android libraries
    find_library(LOG_LIBRARY log)
    find_library(ANDROID_LIBRARY android)
    find_library(EGL_LIBRARY EGL)
    find_library(GLESv2_LIBRARY GLESv2)
    target_link_libraries(gltron PRIVATE
        ${LOG_LIBRARY}
        ${ANDROID_LIBRARY}
        ${EGL_LIBRARY}
        ${GLESv2_LIBRARY}
        OpenGL::GL
    )
endif()
