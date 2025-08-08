#ifdef __ANDROID__
  #include <GLES2/gl2.h>
  #include <android/log.h>
  #define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "GLTRON", __VA_ARGS__))
#else
  #include <GL/gl.h>
  #include <stdio.h>
  #define LOGI(...) printf(__VA_ARGS__); printf("\n")
#endif

#include <SDL2/SDL.h>

// Game loop functions
void handleGameInput(const SDL_Event *event) {
    LOGI("handleGameInput called: type=%d", event->type);
}
void updateGameLogic(void) {
    LOGI("updateGameLogic called");
}

// GUI loop functions
void handleGUIInput(const SDL_Event *event) {
    LOGI("handleGUIInput called: type=%d", event->type);
}
void updateGUI(void) {
    LOGI("updateGUI called");
}

void drawGUI(void) {
#ifdef __ANDROID__
    // Clear with blue for GUI on Android
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#else
    // Clear with blue for GUI on desktop
    glClearColor(0.2f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
    LOGI("drawGUI called");
}

// Pause menu functions
void handlePauseInput(const SDL_Event *event) {
    LOGI("handlePauseInput called: type=%d", event->type);
}
void updatePauseMenu(void) {
    LOGI("updatePauseMenu called");
}
void drawPauseMenu(void) {
#ifdef __ANDROID__
    glClearColor(0.8f, 0.8f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#else
    glClearColor(0.8f, 0.8f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
    LOGI("drawPauseMenu called");
}

// Configure menu functions
void handleConfigureInput(const SDL_Event *event) {
    LOGI("handleConfigureInput called: type=%d", event->type);
}
void updateConfigureMenu(void) {
    LOGI("updateConfigureMenu called");
}
void drawConfigureMenu(void) {
#ifdef __ANDROID__
    glClearColor(0.2f, 0.8f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#else
    glClearColor(0.2f, 0.8f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
    LOGI("drawConfigureMenu called");
}

// Credits screen functions
void handleCreditsInput(const SDL_Event *event) {
    LOGI("handleCreditsInput called: type=%d", event->type);
}
void updateCreditsScreen(void) {
    LOGI("updateCreditsScreen called");
}
void drawCreditsScreen(void) {
#ifdef __ANDROID__
    glClearColor(0.8f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#else
    glClearColor(0.8f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
    LOGI("drawCreditsScreen called");
}

// Timedemo functions
void handleTimedemoInput(const SDL_Event *event) {
    LOGI("handleTimedemoInput called: type=%d", event->type);
}
void updateTimedemo(void) {
    LOGI("updateTimedemo called");
}
void drawTimedemo(void) {
#ifdef __ANDROID__
    glClearColor(0.2f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#else
    glClearColor(0.2f, 0.8f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
    LOGI("drawTimedemo called");
}