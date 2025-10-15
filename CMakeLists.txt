cmake_minimum_required(VERSION 3.10)

# Early purge of Android/AArch64-only linker flags that might leak from env/toolchains into desktop configs
set(_STRIP_FLAGS "--fix-cortex-a53-843419;-Wl,--fix-cortex-a53-843419")
foreach(var CMAKE_EXE_LINKER_FLAGS_INIT CMAKE_SHARED_LINKER_FLAGS_INIT CMAKE_MODULE_LINKER_FLAGS_INIT
            CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS)
  foreach(bad IN LISTS _STRIP_FLAGS)
    if(DEFINED ${var} AND ${var})
      string(REPLACE "${bad}" "" ${var} "${${var}}")
    endif()
  endforeach()
endforeach()
unset(_STRIP_FLAGS)

# Protect desktop builds from accidentally reusing Android toolchain in this build dir
if(NOT ANDROID)
  foreach(var CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_LINKER)
    if(DEFINED ${var} AND ${var})
      if(${var} MATCHES ".*android.*" OR ${var} MATCHES ".*Android.*" OR ${var} MATCHES ".*ndk/.*/ld\\.lld.*")
        message(FATAL_ERROR "Desktop build detected but Android/NDK toolchain is configured in this build directory (${var}='${${var}}'). Use a fresh build directory and, if cross-compiling, pass -DCMAKE_TOOLCHAIN_FILE=tools/toolchains/x86_64-linux-gnu.cmake")
      endif()
    endif()
  endforeach()
endif()

# Set compiler flags for GLUT headers (desktop only; Android uses NDK sysroot)
if(NOT ANDROID)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I/usr/include")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include")
endif()

# OPTION: Architecture checks (OFF by default)
option(STRICT_ARCH_CHECK "Fail if detected third-party libs do not match expected arch" OFF)

project(gltron)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Suppress OpenGL preference warning
cmake_policy(SET CMP0072 NEW)

# Informative output about platform selection
message(STATUS "Configuring gltron for platform: ${CMAKE_SYSTEM_NAME}")

# Set OpenGL preference to legacy
set(OpenGL_GL_PREFERENCE "LEGACY")

# Enable sound by default
option(USE_SOUND "Enable sound support" ON)

# Enable Steamworks multiplayer
option(USE_STEAMWORKS "Enable Steam multiplayer support" OFF)

# Add SOUND macro definition if USE_SOUND is enabled
if(USE_SOUND)
    add_definitions(-DSOUND)
endif()

# Add Steamworks support if enabled
if(USE_STEAMWORKS)
    add_definitions(-DUSE_STEAMWORKS)
    
    # Set Steamworks SDK path
    set(STEAMWORKS_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/steamworks/sdk")
    
    # Include Steamworks headers
    include_directories(${STEAMWORKS_SDK_PATH}/public)
    
    # Find Steamworks library
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(STEAMWORKS_LIB "${STEAMWORKS_SDK_PATH}/redistributable_bin/win64/steam_api64.lib")
        else()
            set(STEAMWORKS_LIB "${STEAMWORKS_SDK_PATH}/redistributable_bin/steam_api.lib")
        endif()
    elseif(APPLE)
        set(STEAMWORKS_LIB "${STEAMWORKS_SDK_PATH}/redistributable_bin/osx/libsteam_api.dylib")
    else()
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(STEAMWORKS_LIB "${STEAMWORKS_SDK_PATH}/redistributable_bin/linux64/libsteam_api.so")
        else()
            set(STEAMWORKS_LIB "${STEAMWORKS_SDK_PATH}/redistributable_bin/linux32/libsteam_api.so")
        endif()
    endif()
    
    message(STATUS "Steamworks library: ${STEAMWORKS_LIB}")
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
    joystick.c
    joystick_steamdeck.c
    multiplayer_menu.c
    lobby_browser.c
    spectator.c
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

# Add Steamworks C++ source files if enabled
if(USE_STEAMWORKS)
    # Enable C++ for Steam files
    enable_language(CXX)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    set(STEAM_SOURCES
        steam_multiplayer.cpp
        steam_lobby.cpp
        steam_network.cpp
        steam_interface.cpp
    )
    list(APPEND SOURCES ${STEAM_SOURCES})
    
    # Mark Steam sources as C++
    set_source_files_properties(${STEAM_SOURCES} PROPERTIES LANGUAGE CXX)
endif()

# Ensure Android-only sources are not part of desktop build
set_source_files_properties(android_glue.c android_main.c backend_openmpt_android.c PROPERTIES HEADER_FILE_ONLY TRUE)

# Exclude shaders.c from desktop build
set_source_files_properties(shaders.c PROPERTIES HEADER_FILE_ONLY TRUE)

# Ensure C sources are compiled as C (not C++)
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)
if(USE_SOUND)
    list(APPEND SOURCES sound.c)
endif()

# Build target: executable on desktop, shared library on Android for NativeActivity
add_executable(gltron ${SOURCES})

# Include directories
target_include_directories(gltron PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Non-Android (desktop Linux/macOS/Windows)
set(USE_GUI ON)

# Prefer CMake find modules; fall back to common paths if needed
# Option to force x86_64 desktop builds (requires a proper cross toolchain)
option(DESKTOP_FORCE_X86_64 "Require desktop build to target x86_64 (use a cross toolchain on non-x86 hosts)" OFF)

# Sanitize any stray Android/AArch64-only linker flags for desktop
if(NOT ANDROID)
    foreach(flag_var CMAKE_EXE_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS)
        if(DEFINED ${flag_var} AND ${flag_var})
            string(REPLACE "--fix-cortex-a53-843419" "" ${flag_var} "${${flag_var}}")
            string(REPLACE "-Wl,--fix-cortex-a53-843419" "" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

# Find and link X11
find_package(X11 REQUIRED)
if(X11_FOUND)
    target_link_libraries(gltron PRIVATE ${X11_LIBRARIES})
    target_include_directories(gltron PRIVATE ${X11_INCLUDE_DIR})
else()
    message(FATAL_ERROR "X11 development libraries not found. Please install libx11-dev.")
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

# Prefer pkg-config for GL/GLU/GLUT when available; rely on toolchain/pkg-config for correct arch
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(GLUT QUIET glut)
    pkg_check_modules(OPENGL QUIET gl)
    pkg_check_modules(GLU QUIET glu)
endif()

if(OPENGL_FOUND AND GLU_FOUND)
    set(OpenGL_FOUND TRUE)
    set(OpenGL_LIBRARIES ${OPENGL_LDFLAGS} ${GLU_LDFLAGS})
    set(OpenGL_INCLUDE_DIR ${OPENGL_INCLUDE_DIRS} ${GLU_INCLUDE_DIRS})
else()
    find_package(OpenGL QUIET)
endif()

# GLUT headers and library (no hardcoded arch-specific lib dirs)
if(GLUT_FOUND)
    set(GLUT_INCLUDE_DIR ${GLUT_INCLUDE_DIRS})
    set(GLUT_LIBRARY ${GLUT_LDFLAGS})
else()
    find_path(GLUT_INCLUDE_DIR GL/glut.h PATHS /usr/include)
    find_library(GLUT_LIBRARY NAMES glut freeglut)
endif()

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

# If GUI is disabled, avoid compiling GUI sources that use OpenGL fixed function
if(NOT USE_GUI)
    set_source_files_properties(gui.c gui_mouse.c PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

# Link math library
target_link_libraries(gltron PRIVATE m)

# Sound backend for desktop
if(USE_SOUND)
    # Prefer pkg-config for MikMod on desktop/cross builds
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(MIKMOD QUIET mikmod)
    endif()

    if(MIKMOD_FOUND)
        set(MikMod_LIBRARIES ${MIKMOD_LDFLAGS})
        set(MikMod_INCLUDE_DIRS ${MIKMOD_INCLUDE_DIRS})
    else()
        # Add the directory containing FindMikMod.cmake to CMAKE_MODULE_PATH
        list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
        # Find MikMod using the custom FindMikMod.cmake module
        find_package(MikMod REQUIRED)
    endif()

    if(MikMod_FOUND OR MIKMOD_FOUND)
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

# Link Steamworks if enabled
if(USE_STEAMWORKS)
    target_link_libraries(gltron PRIVATE ${STEAMWORKS_LIB} stdc++ pthread)
    # Copy Steam API library to output directory
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            configure_file(${STEAMWORKS_SDK_PATH}/redistributable_bin/win64/steam_api64.dll 
                          ${CMAKE_CURRENT_BINARY_DIR}/steam_api64.dll COPYONLY)
        else()
            configure_file(${STEAMWORKS_SDK_PATH}/redistributable_bin/steam_api.dll 
                          ${CMAKE_CURRENT_BINARY_DIR}/steam_api.dll COPYONLY)
        endif()
    elseif(NOT APPLE)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            configure_file(${STEAMWORKS_SDK_PATH}/redistributable_bin/linux64/libsteam_api.so 
                          ${CMAKE_CURRENT_BINARY_DIR}/libsteam_api.so COPYONLY)
        else()
            configure_file(${STEAMWORKS_SDK_PATH}/redistributable_bin/linux32/libsteam_api.so 
                          ${CMAKE_CURRENT_BINARY_DIR}/libsteam_api.so COPYONLY)
        endif()
    endif()
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
