/* GLTron main entry point - Universal (Linux + Android) */

#include "config.h"

#ifdef ANDROID
#include "android_config.h"
#include "base/nebu_system.h"
#include "base/nebu_assert.h"
#include "base/nebu_debug_memory.h"
#include "base/nebu_util.h"
#include "Nebu_filesystem.h"
#include "Nebu_scripting.h"
#include "game/game.h"
#include "android_audio.h"
#include "android_resolution.h"
#include "android_settings_menu.h"
#else
#include "platform_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "gltron-config.h"

#ifdef ANDROID
// Android main function
int android_main(int argc, char *argv[]) {
    LOGI("GLTron Android starting...");
    
    // Initialize Android systems
    android_audio_init();
    android_resolution_init(1920, 1080); // Default resolution
    android_settings_menu_init();
    
    // Load settings
    android_audio_load_settings();
    android_resolution_load_settings();
    
    LOGI("GLTron Android initialized successfully");
    
    // Main game loop would go here
    // For now, just return success
    return 0;
}

// JNI entry point for Android
#include <jni.h>
JNIEXPORT jint JNICALL
Java_com_gltron_android_MainActivity_nativeMain(JNIEnv *env, jobject thiz) {
    return android_main(0, NULL);
}

#else
// Linux main function
int main(int argc, char *argv[]) {
    printf("GLTron Linux starting...\n");
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        LOGE("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    
    // Initialize OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    // Create window
    SDL_Window* window = SDL_CreateWindow(
        "GLTron",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );
    
    if (!window) {
        LOGE("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Create OpenGL context
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        LOGE("Failed to create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        LOGE("Failed to initialize GLEW");
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    LOGI("GLTron Linux initialized successfully");
    LOGI("OpenGL Version: %s", glGetString(GL_VERSION));
    LOGI("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    // Main game loop
    int running = 1;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = 0;
                    }
                    break;
            }
        }
        
        // Clear screen
        glClearColor(0.0f, 0.0f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Swap buffers
        SDL_GL_SwapWindow(window);
        
        // Small delay
        SDL_Delay(16); // ~60 FPS
    }
    
    // Cleanup
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    printf("GLTron Linux exiting...\n");
    return 0;
}
#endif


