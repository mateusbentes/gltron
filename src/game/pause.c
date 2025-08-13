#include "game/camera.h"
#include "game/engine.h"
#include "game/event.h"
#include "game/game.h"
#include "game/game_data.h"
#include "game/gltron.h"

#include "scripting/scripting.h"

#include "configuration/configuration.h"

#include "video/video.h"

#include "base/nebu_callbacks.h"
#include "base/nebu_system.h"

#include "audio/sound_glue.h"
#include "audio/audio.h"

#include "scripting/nebu_scripting.h"
#include "input/nebu_input_system.h"
#include "video/nebu_console.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern video_level *gWorld;  // Use the correct type for gWorld
extern void video_LoadLevel(void);

// Function prototypes
void displayPause(void);
void displayGame(void);
void mousePause(int buttons, int state, int x, int y);
void mouseMotionPause(int mx, int my);
void idlePause(void);
void keyboardPause(int state, int key, int x, int y);
void initPause(void);
void exitPause(void);
void pauseReshape(int x, int y);
void keyboardPrompt(int state, int key, int x, int y);
void initPrompt(void);
void exitPrompt(void);
void promptReshape(int x, int y);

// Forward declarations for missing functions
void doCameraMovement(void);
void changeDisplay(int display);
void nextCameraType(void);
void doBmpScreenShot(Visual *display);  // Changed parameter type to match video.h
void doPngScreenShot(Visual *display);   // Changed parameter type to match video.h
void game_ResetData(void);
void video_ResetData(void);
void Audio_ResetData(void);

/* very brief - just the pause mode */

// Define the pause callbacks structure
ExtendedCallbacks pauseCallbacks = {
    .base = {
        .display = displayPause,
        .idle = idlePause,
        .keyboard = keyboardPause,
        .init = initPause,
        .exit = exitPause,
        .mouse = mousePause,
        .mouseMotion = mouseMotionPause,
        .reshape = pauseReshape,
        .name = "pause"
    }
};

// Implement the displayPause function
void displayPause(void) {
    fprintf(stderr, "[pause] Displaying pause screen\n");

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);

    // Set up the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw the pause screen background
    glColor3f(0.0f, 0.0f, 0.0f); // Black background
    glBegin(GL_QUADS);
        glVertex2i(0, 0);
        glVertex2i(800, 0);
        glVertex2i(800, 600);
        glVertex2i(0, 600);
    glEnd();

    // Draw the pause text using OpenGL primitives
    glColor3f(1.0f, 1.0f, 1.0f); // White text
    glPushMatrix();
    glTranslatef(350, 300, 0);

    // Draw "GAME PAUSED" text using line segments
    const char* pauseText = "GAME PAUSED";
    float x = 0;
    float y = 0;

    // Define character shapes (simplified)
    for (int i = 0; pauseText[i] != '\0'; i++) {
        glPushMatrix();
        glTranslatef(x, y, 0);

        // Draw each character
        switch(pauseText[i]) {
            case 'G':
                // Draw G
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(20, 0);
                    glVertex2f(20, 30);
                    glVertex2f(0, 30);
                    glVertex2f(0, 15);
                    glVertex2f(10, 15);
                glEnd();
                break;
            case 'A':
                // Draw A
                glBegin(GL_LINE_STRIP);
                    glVertex2f(5, 0);
                    glVertex2f(0, 30);
                    glVertex2f(10, 30);
                    glVertex2f(15, 0);
                glEnd();
                glBegin(GL_LINES);
                    glVertex2f(2, 15);
                    glVertex2f(13, 15);
                glEnd();
                break;
            case 'M':
                // Draw M
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 30);
                    glVertex2f(10, 15);
                    glVertex2f(20, 30);
                    glVertex2f(20, 0);
                glEnd();
                break;
            case 'E':
                // Draw E
                glBegin(GL_LINE_STRIP);
                    glVertex2f(20, 0);
                    glVertex2f(0, 0);
                    glVertex2f(0, 30);
                    glVertex2f(20, 30);
                    glVertex2f(0, 30);
                    glVertex2f(0, 15);
                    glVertex2f(20, 15);
                glEnd();
                break;
            case 'P':
                // Draw P
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 30);
                    glVertex2f(20, 30);
                    glVertex2f(20, 15);
                    glVertex2f(0, 15);
                glEnd();
                break;
            case 'U':
                // Draw U
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 30);
                    glVertex2f(0, 0);
                    glVertex2f(20, 0);
                    glVertex2f(20, 30);
                glEnd();
                break;
            case 'S':
                // Draw S
                glBegin(GL_LINE_STRIP);
                    glVertex2f(20, 0);
                    glVertex2f(0, 0);
                    glVertex2f(0, 15);
                    glVertex2f(20, 15);
                    glVertex2f(20, 30);
                    glVertex2f(0, 30);
                glEnd();
                break;
            case 'D':
                // Draw D
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 30);
                    glVertex2f(15, 30);
                    glVertex2f(20, 25);
                    glVertex2f(20, 5);
                    glVertex2f(15, 0);
                    glVertex2f(0, 0);
                glEnd();
                break;
            case ' ':
                // Space
                x += 10; // Add space between words
                break;
        }

        glPopMatrix();
        x += 25; // Move to next character position
    }

    glPopMatrix();

    // Draw instructions using OpenGL primitives
    glColor3f(1.0f, 1.0f, 1.0f); // White text
    glPushMatrix();
    glTranslatef(250, 250, 0);

    const char* instructions = "Press ESC to return to menu or any other key to continue";
    x = 0;
    y = 0;

    // Draw instructions text
    for (int i = 0; instructions[i] != '\0'; i++) {
        glPushMatrix();
        glTranslatef(x, y, 0);

        // Draw each character (simplified)
        switch(instructions[i]) {
            case 'P':
                // Draw P
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 20);
                    glVertex2f(15, 20);
                    glVertex2f(15, 10);
                    glVertex2f(0, 10);
                glEnd();
                break;
            case 'r':
                // Draw r
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 15);
                    glVertex2f(10, 15);
                    glVertex2f(10, 0);
                glEnd();
                break;
            case 'e':
                // Draw e
                glBegin(GL_LINE_STRIP);
                    glVertex2f(10, 0);
                    glVertex2f(0, 0);
                    glVertex2f(0, 15);
                    glVertex2f(10, 15);
                    glVertex2f(0, 15);
                    glVertex2f(0, 7.5);
                    glVertex2f(10, 7.5);
                glEnd();
                break;
            case 's':
                // Draw s
                glBegin(GL_LINE_STRIP);
                    glVertex2f(10, 0);
                    glVertex2f(0, 0);
                    glVertex2f(0, 7.5);
                    glVertex2f(10, 7.5);
                    glVertex2f(10, 15);
                    glVertex2f(0, 15);
                glEnd();
                break;
            case ' ':
                // Space
                x += 5; // Add space between words
                break;
            case 'E':
                // Draw E
                glBegin(GL_LINE_STRIP);
                    glVertex2f(10, 0);
                    glVertex2f(0, 0);
                    glVertex2f(0, 15);
                    glVertex2f(10, 15);
                    glVertex2f(0, 15);
                    glVertex2f(0, 7.5);
                    glVertex2f(10, 7.5);
                glEnd();
                break;
            case 'S':
                // Draw S
                glBegin(GL_LINE_STRIP);
                    glVertex2f(10, 0);
                    glVertex2f(0, 0);
                    glVertex2f(0, 7.5);
                    glVertex2f(10, 7.5);
                    glVertex2f(10, 15);
                    glVertex2f(0, 15);
                glEnd();
                break;
            case 'C':
                // Draw C
                glBegin(GL_LINE_STRIP);
                    glVertex2f(10, 0);
                    glVertex2f(0, 0);
                    glVertex2f(0, 15);
                    glVertex2f(10, 15);
                glEnd();
                break;
            case 't':
                // Draw t
                glBegin(GL_LINE_STRIP);
                    glVertex2f(5, 0);
                    glVertex2f(5, 15);
                    glVertex2f(0, 15);
                    glVertex2f(10, 15);
                glEnd();
                break;
            case 'o':
                // Draw o
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 7.5);
                    glVertex2f(0, 0);
                    glVertex2f(10, 0);
                    glVertex2f(10, 7.5);
                    glVertex2f(0, 7.5);
                glEnd();
                break;
            case 'm':
                // Draw m
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 15);
                    glVertex2f(5, 10);
                    glVertex2f(10, 15);
                    glVertex2f(10, 0);
                glEnd();
                break;
            case 'n':
                // Draw n
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 15);
                    glVertex2f(10, 0);
                    glVertex2f(10, 15);
                glEnd();
                break;
            case 'u':
                // Draw u
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 10);
                    glVertex2f(10, 10);
                    glVertex2f(10, 0);
                glEnd();
                break;
            case 'a':
                // Draw a
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(5, 10);
                    glVertex2f(10, 0);
                    glVertex2f(10, 5);
                    glVertex2f(0, 5);
                glEnd();
                break;
            case 'y':
                // Draw y
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(5, 5);
                    glVertex2f(10, 0);
                    glVertex2f(5, 5);
                    glVertex2f(5, 15);
                glEnd();
                break;
            case 'k':
                // Draw k
                glBegin(GL_LINE_STRIP);
                    glVertex2f(0, 0);
                    glVertex2f(0, 15);
                    glVertex2f(10, 15);
                    glVertex2f(0, 7.5);
                    glVertex2f(10, 0);
                glEnd();
                break;
            case '|':
                // Draw |
                glBegin(GL_LINES);
                    glVertex2f(5, 0);
                    glVertex2f(5, 15);
                glEnd();
                break;
        }

        glPopMatrix();
        x += 10; // Move to next character position
    }

    glPopMatrix();

    // Swap buffers to display the pause screen
    // Using glFinish() as a substitute for glutSwapBuffers()
    glFinish();
}

// Implement the mousePause function
void mousePause(int buttons, int state, int x, int y) {
    fprintf(stderr, "[pause] Mouse event: buttons=%d, state=%d, x=%d, y=%d\n", buttons, state, x, y);

    // Handle mouse events during pause
    if (state == 0) { // Button pressed
        // You can add mouse interaction logic here if needed
    } else { // Button released
        // You can add mouse interaction logic here if needed
    }
}

// Implement the mouseMotionPause function
void mouseMotionPause(int mx, int my) {
    fprintf(stderr, "[pause] Mouse motion: mx=%d, my=%d\n", mx, my);

    // Handle mouse motion during pause
    // You can add mouse motion logic here if needed
}

void idlePause(void) {
    Sound_idle();
    // Allow the game to run by not setting dt to 0
    // Instead, let's update the time but at a slower rate
    if (game2 && game2->time.dt > 0) {
        // Slow down time while paused (optional)
        // game2->time.dt = game2->time.dt / 2;
    }

    doCameraMovement();
    {
        int dx, dy;
        nebu_Input_Mouse_GetDelta(&dx,&dy);
        if(dx || dy)
            nebu_Input_Mouse_WarpToOrigin();
    }
    scripting_RunGC();
    nebu_Time_FrameDelay(10);
    nebu_System_PostRedisplay();
}

void keyboardPause(int state, int key, int x, int y) {
    fprintf(stderr, "[debug] keyboardPause: state=%d, key=%d\n", state, key);

    if(state == NEBU_INPUT_KEYSTATE_UP)
        return;

    switch(key) {
    case 27:
        fprintf(stderr, "[debug] keyboardPause: ESC key pressed, exiting with eSRC_Pause_Escape\n");
        nebu_System_ExitLoop(eSRC_Pause_Escape);
        break;
    case SYSTEM_KEY_F1: changeDisplay(0); break;
    case SYSTEM_KEY_F2: changeDisplay(1); break;
    case SYSTEM_KEY_F3: changeDisplay(2); break;
    case SYSTEM_KEY_F4: changeDisplay(3); break;

        // somehow, this breaks the 'keys' array, and saving
        // at the end of the game fails
        // case SYSTEM_KEY_F5: saveSettings(); return;

    case SYSTEM_KEY_F10: nextCameraType(); break;

    case SYSTEM_KEY_F11: doBmpScreenShot(gScreen); break;
    case SYSTEM_KEY_F12: doPngScreenShot(gScreen); break;

    case SYSTEM_KEY_UP: console_Seek(-1); break;
    case SYSTEM_KEY_DOWN: console_Seek(1); break;

    case SYSTEM_KEY_TAB:
        // nebu_System_ExitLoop(RETURN_MENU_PROMPT);
        break;

    default:
        fprintf(stderr, "[debug] keyboardPause: default case, key=%d\n", key);
        if(game->pauseflag == PAUSE_GAME_FINISHED) {
            fprintf(stderr, "[debug] keyboardPause: game finished, resetting data\n");
            game_ResetData();
            video_ResetData();
            Audio_ResetData();
        }
        else
        {
            fprintf(stderr, "[debug] keyboardPause: unpausing game\n");
            game->pauseflag = PAUSE_GAME_RUNNING;
            nebu_System_ExitLoop(eSRC_Game_Unpause);
        }
        /* lasttime = SystemGetElapsedTime(); */
        break;
    }
}

void initPause(void) {
    /* Check if gWorld is NULL and initialize it if needed */
    if (!gWorld) {
        fprintf(stderr, "[status] initPause: loading level\n");
        video_LoadLevel();

        /* Check if initialization was successful */
        if (!gWorld) {
            fprintf(stderr, "[error] initPause: failed to load level\n");
            return;
        }
    }

    /* Make sure game is properly initialized */
    if (!game) {
        fprintf(stderr, "[error] initPause: game is NULL\n");
        return;
    }

    /* Set the pause flag to indicate we're in pause mode */
    game->pauseflag = PAUSE_GAME_SUSPENDED;

    nebu_Input_HidePointer();
    nebu_Input_Mouse_WarpToOrigin();

    /* disable game sound effects */
    Audio_DisableEngine();

    /* 
    * TODO: Provide an option to disable game music here.
    * Game should be totally silent in pause mode. (Nice when
    * the boss is walking by, phone call, etc...)
    */

    updateSettingsCache();
}

void exitPause(void) {
    /* If we're exiting pause mode to go to game mode, make sure the game is running */
    if (game) {
        game->pauseflag = PAUSE_GAME_RUNNING;
    }

    /* Re-enable game sound effects */
    Audio_EnableEngine();
}

void pauseReshape(int x, int y) {
    /* This function can be empty or handle window resizing if needed */
    fprintf(stderr, "[pause] Reshape: x=%d, y=%d\n", x, y);
}

void keyboardPrompt(int state, int key, int x, int y) {
    if(state == NEBU_INPUT_KEYSTATE_UP)
        return;

    switch(key) {
    case 27:
    case SYSTEM_KEY_TAB:
        nebu_System_ExitLoop(eSRC_Pause_Escape);
        break;
    case SYSTEM_KEY_RETURN:
        /* promptEvaluate(); */
        break;
    }
}

void initPrompt(void) {
    fprintf(stderr, "[pause] Initializing prompt\n");
}

void exitPrompt(void) {
    fprintf(stderr, "[pause] Exiting prompt\n");
}

/* Define a reshape function for prompt callbacks */
void promptReshape(int x, int y) {
    /* This function can be empty or handle window resizing if needed */
    fprintf(stderr, "[pause] Prompt reshape: x=%d, y=%d\n", x, y);
}

// Define the prompt callbacks structure
ExtendedCallbacks promptCallbacks = {
    .base = {
        .display = displayGame,  // Assuming displayGame is defined elsewhere
        .idle = idlePause,
        .keyboard = keyboardPrompt,
        .init = initPrompt,
        .exit = exitPrompt,
        .mouse = NULL,  // No mouse handling for prompts
        .mouseMotion = NULL,  // No mouse motion handling for prompts
        .reshape = promptReshape,
        .name = "prompt"
    }
};
