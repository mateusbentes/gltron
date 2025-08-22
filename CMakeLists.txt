cmake_minimum_required(VERSION 3.10)

# Set compiler flags for GLUT headers (desktop only; Android uses NDK sysroot)
if(NOT ANDROID)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/usr/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include")
endif()

# OPTION: Architecture checks (OFF by default)
option(STRICT_ARCH_CHECK "Fail if detected third-party libs do not match expected arch" OFF)

# Platform-specific settings - must be before project() if Android
if(ANDROID)
    # Set Android build parameters before including the toolchain
    set(ANDROID_ABI "arm64-v8a" CACHE STRING "Android ABI to build")
    set(ANDROID_PLATFORM "29" CACHE STRING "Android API level")

    # Set Android NDK path and include the Android toolchain
    set(ANDROID_NDK $ENV{ANDROID_NDK})
    if(NOT EXISTS "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
        message(FATAL_ERROR "Android NDK not found at ${ANDROID_NDK}. Please set ANDROID_NDK to your NDK path.")
    endif()

    include(${ANDROID_NDK}/build/cmake/android.toolchain.cmake)

    # Set the correct path for Android libraries based on ABI and platform
    if(ANDROID_ABI STREQUAL "arm64-v8a")
        set(ANDROID_LIB_PATH "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/${ANDROID_PLATFORM}")
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(ANDROID_LIB_PATH "${ANDROID_NDK}/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/arm-linux-androideabi/${ANDROID_PLATFORM}")
    else()
        message(FATAL_ERROR "Unsupported ANDROID_ABI '${ANDROID_ABI}'. Use arm64-v8a or armeabi-v7a.")
    endif()

    message(STATUS "Looking for Android libraries in: ${ANDROID_LIB_PATH}")

    # Find Android libraries in the correct path
    find_library(LOG_LIBRARY log PATHS ${ANDROID_LIB_PATH} NO_DEFAULT_PATH)
    find_library(ANDROID_LIBRARY android PATHS ${ANDROID_LIB_PATH} NO_DEFAULT_PATH)
    find_library(EGL_LIBRARY EGL PATHS ${ANDROID_LIB_PATH} NO_DEFAULT_PATH)
    find_library(GLESv2_LIBRARY GLESv2 PATHS ${ANDROID_LIB_PATH} NO_DEFAULT_PATH)
    find_library(OPENSLES_LIB OpenSLES PATHS ${ANDROID_LIB_PATH} NO_DEFAULT_PATH)

    # Verify libraries were found
    foreach(lib LOG_LIBRARY ANDROID_LIBRARY EGL_LIBRARY GLESv2_LIBRARY OPENSLES_LIB)
        if(NOT ${lib})
            message(FATAL_ERROR "${lib} not found in ${ANDROID_LIB_PATH}")
        endif()
    endforeach()
endif()

project(gltron)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Suppress OpenGL preference warning
cmake_policy(SET CMP0072 NEW)

# Informative output about platform selection
message(STATUS "Configuring gltron for platform: ${CMAKE_SYSTEM_NAME}")
if(ANDROID)
    message(STATUS "Android build detected: target ABI=${ANDROID_ABI}; linking to log/android/EGL/GLESv2")
else()
    message(STATUS "Non-Android (desktop) build: using desktop OpenGL and GLUT/GLU if available")
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
        ${OPENSLES_LIB}
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
    )

    # Add NDK include paths
    target_include_directories(gltron PRIVATE
        ${ANDROID_NDK}/sysroot/usr/include
        ${ANDROID_NDK}/sysroot/usr/include/${ANDROID_PLATFORM}
        ${ANDROID_NDK}/sources/android/native_app_glue
    )

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

    # Sound backend for Android
    if(USE_SOUND)
        target_compile_definitions(gltron PRIVATE SOUND_BACKEND_OPENMPT)

        # Define paths for OpenMPT library and headers
        set(OPENMPT_PATHS
            "${CMAKE_SOURCE_DIR}/android-dependencies/arm64-v8a/prefix"
            "${CMAKE_SOURCE_DIR}/android-dependencies/${ANDROID_ABI}/prefix"
            "${CMAKE_BINARY_DIR}/openmpt-install"
            "/usr"
            "/usr/local"
        )

        # First try to use the prebuilt library from android-dependencies
        set(OPENMPT_LIBRARY "${CMAKE_SOURCE_DIR}/android-dependencies/arm64-v8a/prefix/lib/libopenmpt.a")
        set(OPENMPT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/android-dependencies/arm64-v8a/prefix/include")

        # Verify the prebuilt library exists
        if(EXISTS "${OPENMPT_LIBRARY}" AND EXISTS "${OPENMPT_INCLUDE_DIR}/libopenmpt/libopenmpt.h")
            message(STATUS "Using prebuilt OpenMPT from: ${OPENMPT_LIBRARY}")

            # Verify library architecture
            execute_process(
                COMMAND file ${OPENMPT_LIBRARY}
                OUTPUT_VARIABLE LIB_ARCH
            )

            if(NOT LIB_ARCH MATCHES "ARM aarch64")
                message(WARNING "OpenMPT library is not for ARM64 architecture. Found: ${LIB_ARCH}")
                # Continue with the prebuilt library anyway
            endif()
        else()
            # If prebuilt library not found, try to build it from source with correct architecture
            message(STATUS "OpenMPT library not found. Attempting to build from source...")

            # Create a custom target to build OpenMPT with correct architecture
            add_custom_target(build_openmpt
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/openmpt-build
                COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_BINARY_DIR}/openmpt-build
                    ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/openmpt-install
                    -DCMAKE_BUILD_TYPE=Release
                    -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake
                    -DANDROID_ABI=arm64-v8a
                    -DANDROID_PLATFORM=29
                    -DCMAKE_C_FLAGS="-march=armv8-a"
                    -DCMAKE_CXX_FLAGS="-march=armv8-a"
                    ${CMAKE_CURRENT_SOURCE_DIR}/openmpt
                COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/openmpt-build --target install
                COMMENT "Building OpenMPT from source for ARM64"
            )

            # Add dependency to ensure OpenMPT is built before linking
            add_dependencies(gltron build_openmpt)

            # Set paths to the built OpenMPT
            set(OPENMPT_INCLUDE_DIR "${CMAKE_BINARY_DIR}/openmpt-install/include")
            set(OPENMPT_LIBRARY "${CMAKE_BINARY_DIR}/openmpt-install/lib/libopenmpt.a")
        endif()

        # Find OpenMPT headers if not already set
        if(NOT OPENMPT_INCLUDE_DIR)
            find_path(OPENMPT_INCLUDE_DIR
                NAMES libopenmpt/libopenmpt.h
                PATHS ${OPENMPT_PATHS}
                PATH_SUFFIXES include
            )

            if(NOT OPENMPT_INCLUDE_DIR)
                message(FATAL_ERROR "OpenMPT headers not found in any of these locations:\n${OPENMPT_PATHS}\nPlease install libopenmpt-dev or provide the correct path to OpenMPT headers.")
            endif()
        endif()

        # Verify library exists before linking
        if(NOT EXISTS "${OPENMPT_LIBRARY}")
            message(FATAL_ERROR "OpenMPT library not found at ${OPENMPT_LIBRARY}")
        endif()

        # Configure the target with OpenMPT
        target_include_directories(gltron PRIVATE ${OPENMPT_INCLUDE_DIR})
        target_link_libraries(gltron PRIVATE ${OPENMPT_LIBRARY})

        message(STATUS "Using OpenMPT from: ${OPENMPT_LIBRARY}")
    endif()

else()
    # Non-Android settings (desktop)
    set(USE_GUI ON)

    # Prefer CMake find modules; fall back to common paths if needed
    find_package(OpenGL QUIET)
    if(NOT OpenGL_FOUND)
        # Fallback to typical x86_64 Linux paths
        find_library(OPENGL_LIBRARY NAMES GL libGL PATHS /usr/lib/x86_64-linux-gnu)
        find_library(GLU_LIBRARY NAMES GLU libGLU PATHS /usr/lib/x86_64-linux-gnu)
        if(OPENGL_LIBRARY AND GLU_LIBRARY)
            set(OpenGL_FOUND TRUE)
            set(OpenGL_LIBRARIES ${OPENGL_LIBRARY} ${GLU_LIBRARY})
        endif()
    endif()

    # GLUT headers and library
    find_path(GLUT_INCLUDE_DIR GL/glut.h PATHS /usr/include)
    find_library(GLUT_LIBRARY NAMES glut freeglut PATHS /usr/lib/x86_64-linux-gnu)

    if(GLUT_INCLUDE_DIR)
        message(STATUS "Found GLUT headers at: ${GLUT_INCLUDE_DIR}")
        set(GLUT_FOUND TRUE)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${GLUT_INCLUDE_DIR}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${GLUT_INCLUDE_DIR}")
    else()
        message(WARNING "Could not find GLUT headers. Game will run without GUI.")
        set(USE_GUI OFF)
    endif()

    if(USE_GUI)
        if(GLUT_FOUND AND OpenGL_FOUND AND GLUT_LIBRARY)
            target_link_libraries(gltron PRIVATE ${OpenGL_LIBRARIES} ${GLUT_LIBRARY})
        else()
            message(WARNING "GUI dependencies not found. Game will run without GUI.")
            set(USE_GUI OFF)
        endif()
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
            if(STRICT_ARCH_CHECK)
                # Verify MikMod library architecture (optional)
                execute_process(
                    COMMAND file ${MikMod_LIBRARIES}
                    OUTPUT_VARIABLE LIB_ARCH
                )
                if(NOT LIB_ARCH MATCHES "x86-64")
                    message(FATAL_ERROR "MikMod library is not for x86_64 architecture. Found: ${LIB_ARCH}")
                endif()
            endif()

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

# Note: Additional global MikMod discovery block removed to avoid duplication. Desktop sound is configured above.

# Android OpenMPT configuration is already handled in the main ANDROID block above; duplicate removed.

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

# Platform-specific settings (end)
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
endif()
