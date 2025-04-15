/* Implementation of nebu_System_SetCallback_* functions */

#include "base/nebu_system.h"
#include <stdio.h>

/* Callback function pointers */
static void (*display_callback)(void) = NULL;
static void (*idle_callback)(void) = NULL;
static void (*keyboard_callback)(int state, int key, int x, int y) = NULL;
static void (*mouse_callback)(int button, int state, int x, int y) = NULL;
static void (*mouse_motion_callback)(int x, int y) = NULL;
static void (*system_event_callback)(void *event) = NULL;

/* Set display callback */
void nebu_System_SetCallback_Display(void (*callback)(void)) {
    printf("[system] Setting display callback to %p\n", callback);
    display_callback = callback;
}

/* Set idle callback */
void nebu_System_SetCallback_Idle(void (*callback)(void)) {
    printf("[system] Setting idle callback to %p\n", callback);
    idle_callback = callback;
}

/* Set keyboard callback */
void nebu_System_SetCallback_Key(void (*callback)(int state, int key, int x, int y)) {
    printf("[system] Setting keyboard callback to %p\n", callback);
    keyboard_callback = callback;
}

/* Set mouse callback */
void nebu_System_SetCallback_Mouse(void (*callback)(int button, int state, int x, int y)) {
    printf("[system] Setting mouse callback to %p\n", callback);
    mouse_callback = callback;
}

/* Set mouse motion callback */
void nebu_System_SetCallback_MouseMotion(void (*callback)(int x, int y)) {
    printf("[system] Setting mouse motion callback to %p\n", callback);
    mouse_motion_callback = callback;
}

/* Set system event callback */
void nebu_System_SetCallback_SystemEvent(void (*callback)(void *event)) {
    printf("[system] Setting system event callback to %p\n", callback);
    system_event_callback = callback;
}

/* Call display callback */
void nebu_System_Display(void) {
    if (display_callback) {
        display_callback();
    }
}

/* Call idle callback */
void nebu_System_Idle(void) {
    if (idle_callback) {
        idle_callback();
    }
}

/* Call keyboard callback */
void nebu_System_Key(int state, int key, int x, int y) {
    if (keyboard_callback) {
        keyboard_callback(state, key, x, y);
    }
}

/* Call mouse callback */
void nebu_System_Mouse(int button, int state, int x, int y) {
    if (mouse_callback) {
        mouse_callback(button, state, x, y);
    }
}

/* Call mouse motion callback */
void nebu_System_MouseMotion(int x, int y) {
    if (mouse_motion_callback) {
        mouse_motion_callback(x, y);
    }
}

/* Call system event callback */
void nebu_System_SystemEvent(void *event) {
    if (system_event_callback) {
        system_event_callback(event);
    }
}

/* Main loop implementation */
void nebu_System_MainLoop(void) {
    printf("[system] Entering main loop\n");
    
    /* Simple main loop implementation */
    int running = 1;
    while (running) {
        /* Process events */
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                    
                case SDL_KEYDOWN:
                    nebu_System_Key(1, event.key.keysym.sym, 0, 0);
                    break;
                    
                case SDL_KEYUP:
                    nebu_System_Key(0, event.key.keysym.sym, 0, 0);
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    nebu_System_Mouse(event.button.button, 1, event.button.x, event.button.y);
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    nebu_System_Mouse(event.button.button, 0, event.button.x, event.button.y);
                    break;
                    
                case SDL_MOUSEMOTION:
                    nebu_System_MouseMotion(event.motion.x, event.motion.y);
                    break;
                    
                /* Add touch event handling for mobile platforms */
                #if SDL_VERSION_ATLEAST(2,0,0)
                case SDL_FINGERDOWN:
                case SDL_FINGERUP:
                case SDL_FINGERMOTION:
                    {
                        /* Create a system event and pass it to the callback */
                        typedef struct {
                            int type;
                            union {
                                struct {
                                    float x, y;
                                } tfinger;
                            };
                        } SystemEvent;
                        
                        SystemEvent sysEvent;
                        
                        if (event.type == SDL_FINGERDOWN) {
                            sysEvent.type = 1; /* SYSTEM_FINGERDOWN */
                        } else if (event.type == SDL_FINGERUP) {
                            sysEvent.type = 2; /* SYSTEM_FINGERUP */
                        } else {
                            sysEvent.type = 3; /* SYSTEM_FINGERMOTION */
                        }
                        
                        sysEvent.tfinger.x = event.tfinger.x;
                        sysEvent.tfinger.y = event.tfinger.y;
                        
                        nebu_System_SystemEvent(&sysEvent);
                    }
                    break;
                #endif
            }
        }
        
        /* Call idle callback */
        nebu_System_Idle();
        
        /* Call display callback */
        nebu_System_Display();
        
        /* Limit frame rate */
        SDL_Delay(16); /* ~60 FPS */
    }
    
    printf("[system] Exiting main loop\n");
}

/* Exit function */
void nebu_System_Exit(void) {
    printf("[system] Exiting system\n");
    exit(0);
}