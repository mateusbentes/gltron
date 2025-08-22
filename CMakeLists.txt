cmake_minimum_required(VERSION 3.10)

# Platform-specific settings - must be before project() if Android
if(ANDROID)
    # Set Android NDK path and include the Android toolchain
    set(ANDROID_NDK $ENV{ANDROID_NDK})
    if(NOT EXISTS "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
        message(FATAL_ERROR "Android NDK not found at ${ANDROID_NDK}. Please set ANDROID_NDK to your NDK path.")
    endif()

    include(${ANDROID_NDK}/build/cmake/android.toolchain.cmake)

    # Set up Android platform and ABI
    set(ANDROID_ABI arm64-v8a)  # or armeabi-v7a
    set(ANDROID_PLATFORM android-29)  # Set to minimum supported version

    # Set appropriate compiler flags for Android
    if(ANDROID_ABI STREQUAL "arm64-v8a")
        # Use appropriate architecture flags for ARM64
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv8-a")
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        # Use appropriate architecture flags for ARMv7
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=softfp")
    endif()

    # Additional Android-specific compiler flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
endif()

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

# Prefer prebuilt OpenMPT from android-dependencies if building for Android
if(ANDROID)
    set(_OPENMPT_DEFAULT_ABI "arm64-v8a")
    if(DEFINED ANDROID_ABI AND NOT ANDROID_ABI STREQUAL "")
        set(_OPENMPT_ABI "${ANDROID_ABI}")
    else()
        set(_OPENMPT_ABI "${_OPENMPT_DEFAULT_ABI}")
    endif()
    set(_OPENMPT_PREFIX "${CMAKE_SOURCE_DIR}/android-dependencies/${_OPENMPT_ABI}/prefix")
    set(_OPENMPT_LIB "${_OPENMPT_PREFIX}/lib/libopenmpt.a")
    set(_OPENMPT_INC "${_OPENMPT_PREFIX}/include")
    if(EXISTS "${_OPENMPT_LIB}" AND EXISTS "${_OPENMPT_INC}")
        message(STATUS "Prebuilt OpenMPT detected at: ${_OPENMPT_LIB}")
        set(OpenMPT_LIBRARIES "${_OPENMPT_LIB}" CACHE FILEPATH "Path to libopenmpt.a")
        set(OpenMPT_INCLUDE_DIRS "${_OPENMPT_INC}" CACHE PATH "Path to OpenMPT headers")
        # Also set alternative variable names used later in the file
        set(OpenMPT_LIBRARY "${_OPENMPT_LIB}" CACHE FILEPATH "Path to libopenmpt.a")
        set(OpenMPT_INCLUDE_DIR "${_OPENMPT_INC}" CACHE PATH "Path to OpenMPT headers")
        # Help CMake find these prebuilt deps if any sub-find_package is used
        set(CMAKE_PREFIX_PATH "${_OPENMPT_PREFIX};${CMAKE_PREFIX_PATH}" CACHE STRING "" FORCE)
        include_directories(BEFORE SYSTEM "${_OPENMPT_INC}")
        link_directories(BEFORE "${_OPENMPT_PREFIX}/lib")
    endif()
endif()

# Add SOUND macro definition if USE_SOUND is enabled
if(USE_SOUND)
    add_definitions(-DSOUND)
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

# Add shader source files only for Android
if(ANDROID)
    set(SHADER_SOURCES shaders.c)
    list(APPEND SOURCES ${SHADER_SOURCES})

    # Ensure shader sources are compiled as C (not C++)
    set_source_files_properties(${SHADER_SOURCES} PROPERTIES LANGUAGE C)
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

# Platform-specific settings after target creation
if(ANDROID)
    # Define ANDROID macro for C sources
    target_compile_definitions(gltron PRIVATE ANDROID)

    # Link Android libraries
    find_library(LOG_LIBRARY log)
    find_library(ANDROID_LIBRARY android)
    find_library(EGL_LIBRARY EGL)
    find_library(GLESv2_LIBRARY GLESv2)

    # Link Android libraries to the target
    target_link_libraries(gltron PRIVATE
        ${LOG_LIBRARY}
        ${ANDROID_LIBRARY}
        ${EGL_LIBRARY}
        ${GLESv2_LIBRARY}
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )

    # Add NDK include paths
    target_include_directories(gltron PRIVATE
        ${ANDROID_NDK}/sysroot/usr/include
        ${ANDROID_NDK}/sysroot/usr/include/${ANDROID_PLATFORM}
        ${ANDROID_NDK}/sources/android/native_app_glue
    )

    # Sound backend for Android
    if(USE_SOUND)
        target_compile_definitions(gltron PRIVATE SOUND_BACKEND_OPENMPT)
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
        target_sources(gltron PRIVATE backend_openmpt_android.c)
    endif()
else()
    # Non-Android settings
    set(USE_GUI ON)

    # Try to find GLUT with multiple package names
    find_package(GLUT QUIET)
    if(NOT GLUT_FOUND)
        find_package(glut QUIET)
        if(NOT glut_FOUND)
            find_package(freeglut QUIET)
            if(NOT freeglut_FOUND)
                message(WARNING "Could not find GLUT or freeglut. Game will run without GUI.")
                set(USE_GUI OFF)
            endif()
        endif()
    endif()

    # Link libraries
    find_package(OpenGL REQUIRED)
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

    # Link math library
    target_link_libraries(gltron PRIVATE m)

    # Sound backend for desktop
    if(USE_SOUND)
        # Add the directory containing FindMikMod.cmake to CMAKE_MODULE_PATH
        list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

        # Find MikMod using the custom FindMikMod.cmake module
        find_package(MikMod REQUIRED)

        if(MikMod_FOUND)
            target_link_libraries(gltron PRIVATE ${MikMod_LIBRARIES})
            target_include_directories(gltron PRIVATE ${MikMod_INCLUDE_DIRS})
            target_sources(gltron PRIVATE backend_mikmod.c)

            # Link additional libraries for WAV support
            if(USE_WAV_SUPPORT)
                find_package(Vorbis QUIET)
                if(Vorbis_FOUND)
                    target_link_libraries(gltron PRIVATE ${Vorbis_LIBRARIES})
                else()
                    message(WARNING "Could not find Vorbis. WAV file support will be limited.")
                endif()

                find_package(FLAC QUIET)
                if(FLAC_FOUND)
                    target_link_libraries(gltron PRIVATE ${FLAC_LIBRARIES})
                else()
                    message(WARNING "Could not find FLAC. WAV file support will be limited.")
                endif()

                find_package(Ogg QUIET)
                if(Ogg_FOUND)
                    target_link_libraries(gltron PRIVATE ${Ogg_LIBRARIES})
                else()
                    message(WARNING "Could not find Ogg. WAV file support will be limited.")
                endif()
            endif()
        else()
            message(FATAL_ERROR "MikMod library not found. Please install libmikmod-dev or provide the path to MikMod.")
        endif()
    endif()
endif()

# Find MikMod and related packages for sound support
if(USE_SOUND AND NOT ANDROID)
    # Add the directory containing FindMikMod.cmake to CMAKE_MODULE_PATH
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

    # Find MikMod using the custom FindMikMod.cmake module
    find_package(MikMod REQUIRED)

    if(MikMod_FOUND)
        message(STATUS "Found MikMod: ${MikMod_LIBRARIES}")
        target_link_libraries(gltron PRIVATE ${MikMod_LIBRARIES})
        target_include_directories(gltron PRIVATE ${MikMod_INCLUDE_DIRS})

        if(USE_WAV_SUPPORT)
            # Try multiple package names for Vorbis
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

            # Try multiple package names for FLAC
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

            # Try multiple package names for Ogg
            find_package(Ogg QUIET)
            if(NOT Ogg_FOUND)
                find_package(ogg QUIET)
                if(NOT ogg_FOUND)
                    find_package(libogg QUIET)
                    if(NOT libogg_FOUND)
                        message(WARNING "Could not find Ogg. WAV file support will be limited.")
                        set(USE_WAV_SUPPORT OFF)
                    endif()
                endif()
            endif()
        endif()
    else()
        message(FATAL_ERROR "MikMod library not found. Please install libmikmod-dev or provide the path to MikMod.")
    endif()
endif()

if(ANDROID AND USE_SOUND)
    # Use preconfigured include/library if already provided (from prebuilt detection)
    if(DEFINED OpenMPT_INCLUDE_DIRS AND EXISTS "${OpenMPT_INCLUDE_DIRS}")
        set(OpenMPT_INCLUDE_DIR "${OpenMPT_INCLUDE_DIRS}")
        message(STATUS "Using preconfigured OpenMPT headers at: ${OpenMPT_INCLUDE_DIR}")
        target_include_directories(gltron PRIVATE ${OpenMPT_INCLUDE_DIR})
    else()
        # Try to find headers in common locations, including android-dependencies prefix
        find_path(OpenMPT_INCLUDE_DIR libopenmpt/libopenmpt.h
            PATHS
            ${CMAKE_SOURCE_DIR}/android-dependencies/${ANDROID_ABI}/prefix/include
            ${CMAKE_SOURCE_DIR}/android-dependencies/arm64-v8a/prefix/include
            /usr/include
            /usr/local/include
            /usr/include/libopenmpt
            ${ANDROID_NDK}/sources/third_party/openmpt/include
        )
        if(OpenMPT_INCLUDE_DIR)
            message(STATUS "Found OpenMPT headers at: ${OpenMPT_INCLUDE_DIR}")
            target_include_directories(gltron PRIVATE ${OpenMPT_INCLUDE_DIR})
        else()
            message(FATAL_ERROR "OpenMPT headers not found. Please install libopenmpt-dev or provide the path to OpenMPT headers.")
        endif()
    endif()

    # Prefer preconfigured library if available
    if(DEFINED OpenMPT_LIBRARIES AND EXISTS "${OpenMPT_LIBRARIES}")
        set(OpenMPT_LIBRARY "${OpenMPT_LIBRARIES}")
        message(STATUS "Using preconfigured OpenMPT library at: ${OpenMPT_LIBRARY}")
        target_link_libraries(gltron PRIVATE ${OpenMPT_LIBRARY})
    else()
        # Find OpenMPT library in common locations
        find_library(OpenMPT_LIBRARY openmpt
            PATHS
            ${CMAKE_SOURCE_DIR}/android-dependencies/${ANDROID_ABI}/prefix/lib
            ${CMAKE_SOURCE_DIR}/android-dependencies/arm64-v8a/prefix/lib
            /usr/lib
            /usr/local/lib
            /usr/lib/x86_64-linux-gnu
            ${ANDROID_NDK}/sources/third_party/openmpt/lib
        )
        if(OpenMPT_LIBRARY)
            message(STATUS "Found OpenMPT library at: ${OpenMPT_LIBRARY}")
            target_link_libraries(gltron PRIVATE ${OpenMPT_LIBRARY})
        else()
            # If library not found, try to build it from source
            message(STATUS "OpenMPT library not found. Attempting to build from source...")

            # Create a custom target to build OpenMPT
            add_custom_target(build_openmpt
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/openmpt-build
                COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/openmpt-build
                    ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/openmpt-install
                    -DCMAKE_BUILD_TYPE=Release
                    ${CMAKE_CURRENT_SOURCE_DIR}/openmpt
                COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/openmpt-build --target install
                COMMENT "Building OpenMPT from source"
            )

            # Add dependency to ensure OpenMPT is built before linking
            add_dependencies(gltron build_openmpt)

            # Set paths to the built OpenMPT
            set(OpenMPT_INCLUDE_DIR ${CMAKE_BINARY_DIR}/openmpt-install/include)
            set(OpenMPT_LIBRARY ${CMAKE_BINARY_DIR}/openmpt-install/lib/libopenmpt.a)
            message(STATUS "Using built OpenMPT from: ${OpenMPT_LIBRARY}")
            target_include_directories(gltron PRIVATE ${OpenMPT_INCLUDE_DIR})
            target_link_libraries(gltron PRIVATE ${OpenMPT_LIBRARY})
        endif()
    endif()
endif()

# Platform-specific settings
if(WIN32)
    target_link_libraries(gltron PRIVATE opengl32 glu32)
    if(USE_SOUND)
        target_link_libraries(gltron PRIVATE winmm)
    endif()
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(gltron PRIVATE ${COCOA_LIBRARY})
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
