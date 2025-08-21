/*
  gltron 0.50 beta
  Copyright (C) 1999 by Andreas Umbach <marvin@dataway.ch>
*/

#include "gltron.h"
#include "globals.h"
#include "model.h"
#include "fonttex.h"
#include "menu.h"
#include "sgi_texture.h"
#include "shaders.h"

#ifdef ANDROID
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// Add this global variable declaration
float projectionMatrix[16];
#ifdef ANDROID
GLuint shaderProgram;
#endif

int getElapsedTime(void) {
#ifdef ANDROID
    // Android implementation using clock_gettime
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec * 1000) + (now.tv_nsec / 1000000);
#elif defined(WIN32)
    // Windows implementation using timeGetTime
    return timeGetTime();
#else
    // Default implementation for other platforms using GLUT
    static int start_time = 0;
    static int initialized = 0;

    if (!initialized) {
        // Initialize the start time on first call
        start_time = glutGet(GLUT_ELAPSED_TIME);
        initialized = 1;
    }

    // Return the elapsed time since initialization
    return glutGet(GLUT_ELAPSED_TIME) - start_time;
#endif
}

void mouseWarp() {
#ifndef ANDROID
    glutWarpPointer(game->screen->w / 2, game->screen->h / 2);
#endif
}

void drawGame() {
  #ifdef ANDROID
    useShaderProgram(shaderProgram);
  #endif

  GLint i;
  gDisplay *d;
  Player *p;

  polycount = 0;
  glClearColor(.0, .0, .0, .0);
  glDepthMask(GL_TRUE);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDepthMask(GL_FALSE);

  for(i = 0; i < vp_max[ game->settings->display_type]; i++) {
    p = &(game->player[ game->settings->content[i] ]);
    if(p->display->onScreen == 1) {
      d = p->display;
      glViewport(d->vp_x, d->vp_y, d->vp_w, d->vp_h);
      drawCam(p, d);
      drawScore(p, d);
      if(game->settings->show_ai_status)
	if(p->ai->active == 1)
	  drawAI(d);
    }
  }

  if(game->settings->show_2d > 0)
    drawDebugTex(game->screen);
  if(game->settings->show_fps)
    drawFPS(game->screen);

  /*
  if(game->settings->show_help == 1)
    drawHelp(game->screen);
  */

  /* printf("%d polys\n", polycount); */
}
void displayGame() {
    /* Ensure viewports match window size at draw time after any display change */
    forceViewportResetIfNeededForGame();
    drawGame();
    if(game->settings->mouse_warp)
        mouseWarp();
#ifndef ANDROID
    glutSwapBuffers();
#endif
}

void initCustomLights() {
#ifndef ANDROID
    float col[] = { .77, .77, .77, 1.0 };
    float dif[] =  { 0.4, 0.4, 0.4, 1};
    float amb[] = { 0.25, 0.25, 0.25, 1};

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
#endif
}

void initGLGame() {
#ifdef ANDROID
    // Android-specific OpenGL initialization
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create shader program
    shaderProgram = createShaderProgram();
#else
    // First create the window if it doesn't exist
    if (glutGetWindow() == 0) {
        glutInitWindowSize(game->settings->width, game->settings->height);
        glutCreateWindow("GLtron");
    }

    printf("OpenGL Info: '%s'\n%s - %s\n", glGetString(GL_VENDOR),
            glGetString(GL_RENDERER), glGetString(GL_VERSION));

    glShadeModel(GL_FLAT);

    if(game->settings->show_alpha)
        glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glFogf(GL_FOG_START, 50.0);
    glFogf(GL_FOG_END, 100.0);
    glFogf(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_DENSITY, 0.1);
    glDisable(GL_FOG);

    // Apply fullscreen mode if needed
    if (game->settings->fullscreen) {
        glutFullScreen();
    }

    initCustomLights();

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
#endif
}

int initWindow() {
#ifdef ANDROID
    // Android-specific window initialization
    return 1;
#else
    int win_id;
    glutInitWindowSize(game->settings->width, game->settings->height);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    win_id = glutCreateWindow("gltron");
    if (win_id < 0) {
        printf("could not create window...exiting\n");
        exit(1);
    }
    return win_id;
#endif
}

void shutdownDisplay(gDisplay *d) {
    deleteTextures();
    deleteFonts();
#ifndef ANDROID
    glutDestroyWindow(d->win_id);
#endif
    printf("window destroyed\n");
}

static int g_pending_display_apply = 0;
static int g_just_applied_display_change = 0;
static int g_apply_cooldown_frames = 0;

void requestDisplayApply() {
  g_pending_display_apply = 1;
}

void forceViewportResetIfNeededForGui() {
    if (!g_just_applied_display_change) return;
#ifndef ANDROID
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    if (w > 0 && h > 0) {
        glViewport(0, 0, w, h);
        guiProjection(w, h);
        printf("GUI viewport/projection reset to %dx%d after display change.\n", w, h);
    }
#endif
    g_just_applied_display_change = 0;
}

void forceViewportResetIfNeededForGame() {
    if (!g_just_applied_display_change) return;

    // Get the current window dimensions
    int w = game->settings->width;
    int h = game->settings->height;

    // Ensure valid dimensions
    if (w <= 0 || h <= 0) {
        // Fallback to default dimensions if invalid
        w = 800;
        h = 600;
    }

    // Set the viewport to cover the new window size
    glViewport(0, 0, w, h);

#ifdef ANDROID
    // For Android, calculate and set the perspective matrix in the shader
    float aspect = (float)w / (float)h;
    float fov = game->settings->fov;
    float near = 0.1f;
    float far = 1000.0f;

    // Calculate the perspective matrix
    float top = near * tanf(fov * 3.14159265f / 360.0f);
    float bottom = -top;
    float left = bottom * aspect;
    float right = top * aspect;

    // Create a perspective matrix
    float matrix[16] = {
        2.0f * near / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f * near / (top - bottom), 0.0f, 0.0f,
        (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near), -1.0f,
        0.0f, 0.0f, -2.0f * far * near / (far - near), 0.0f
    };

    // Set the projection matrix uniform in your shader
    // This would typically be done in your rendering code
    // For example:
    // glUniformMatrix4fv(projectionMatrixLocation, 1, GL_FALSE, matrix);
#else
    // For non-Android platforms, use standard OpenGL functions
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(game->settings->fov, (float)w / (float)h, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
#endif

    // Reinitialize game screen and display
    initGameScreen();
    changeDisplay();

    printf("Game viewport reset to %dx%d after display change.\n", w, h);

    g_just_applied_display_change = 0;
}

void applyDisplaySettingsDeferred() {
    if (!g_pending_display_apply) return;
    g_pending_display_apply = 0;

    if (!game || !game->settings) return;

    /* Ensure we have a window; if not, create one */
    if (!game->screen) {
        game->screen = (gDisplay*) malloc(sizeof(gDisplay));
    }

#ifndef ANDROID
    /* Apply windowed/fullscreen and resolution safely */
    if (game->settings->fullscreen) {
        /* Ensure we are current and then enter fullscreen */
        if (game->screen->win_id <= 0) {
            setupDisplay(game->screen);
        } else {
            glutSetWindow(game->screen->win_id);
            glutFullScreen();
        }

        /* Query actual window size and update settings */
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);
        if (w > 0 && h > 0) {
            game->settings->width = w;
            game->settings->height = h;
            if (game->screen) {
                game->screen->w = w;
                game->screen->h = h;
            }
        }

        /* Reinitialize screen and viewports after entering fullscreen with updated size */
        initGameScreen();
        changeDisplay();
        updateCallbacks();
        printf("Applied fullscreen via deferred apply. Window size: %dx%d.\n", w, h);

        /* Persist new size for next start */
        saveSettings();
    } else {
        /* Windowed: rebuild to requested resolution */
        printf("Applying windowed mode %dx%d via deferred apply.\n", game->settings->width, game->settings->height);

        /* Destroy and recreate window to ensure clean state */
        if (game->screen && game->screen->win_id > 0) {
            shutdownDisplay(game->screen);
        }

        setupDisplay(game->screen);
        initGameScreen();
        changeDisplay();
        updateCallbacks();
    }
#endif

    /* Mark to force viewport/projection reset on next frame and skip one draw */
    g_just_applied_display_change = 1;
    g_apply_cooldown_frames = 1;
}

void onReshape(int w, int h) {
    // Update the game settings with the new window size
    game->settings->width = w;
    game->settings->height = h;

    // Update the display structure
    if (game->screen) {
        game->screen->w = w;
        game->screen->h = h;
    }

    // Set the viewport to cover the new window size
    glViewport(0, 0, w, h);

    // Calculate common perspective parameters
    float aspect = (float)w / (float)h;
    float fov = game->settings->fov;
    float near = 0.1f;
    float far = 1000.0f;

    // Use appropriate perspective function based on platform
#ifdef ANDROID
    // For Android, calculate the perspective matrix
    float top = near * tanf(fov * 3.14159265f / 360.0f);
    float bottom = -top;
    float left = bottom * aspect;
    float right = top * aspect;

    // Create a perspective matrix for OpenGL ES
    float matrix[16] = {
        2.0f * near / (right - left), 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f * near / (top - bottom), 0.0f, 0.0f,
        (right + left) / (right - left), (top + bottom) / (top - bottom), -(far + near) / (far - near), -1.0f,
        0.0f, 0.0f, -2.0f * far * near / (far - near), 0.0f
    };

    // Copy the matrix to the global projection matrix
    memcpy(projectionMatrix, matrix, sizeof(matrix));

    // Set the projection matrix in the shader
    useShaderProgram(shaderProgram);
    setProjectionMatrix(shaderProgram, projectionMatrix);
#else
    // For non-Android platforms, use standard OpenGL functions
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fov, aspect, near, far);
    glMatrixMode(GL_MODELVIEW);
#endif

    // Reinitialize game screen and display
    initGameScreen();
    changeDisplay();

    // Force a redisplay
#ifndef ANDROID
    glutPostRedisplay();
#endif
}

void setupDisplay(gDisplay *d) {
#ifdef ANDROID
    // Android-specific display initialization
    printf("Android display setup\n");
    d->win_id = 1; // Dummy window ID for Android
    printf("loading fonts...\n");
    initFonts();
    printf("loading textures...\n");
    initTexture(d);
    printf("window created with ID: %d\n", d->win_id);
#else
    printf("trying to create window\n");

    // Initialize GLUT window properties
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    // Set initial window size from settings
    glutInitWindowSize(game->settings->width, game->settings->height);

    // Set window position for windowed mode
    glutInitWindowPosition(0, 0);

    // Create the window
    d->win_id = glutCreateWindow("GLtron");

    // Set fullscreen mode if needed
    if (game->settings->fullscreen) {
        glutFullScreen();
        // Query actual window size and update settings
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);
        if (w > 0 && h > 0) {
            game->settings->width = w;
            game->settings->height = h;
            d->w = w;
            d->h = h;
        }
    } else {
        // Ensure window is in windowed mode
        glutReshapeWindow(game->settings->width, game->settings->height);
    }

    printf("window created with ID: %d\n", d->win_id);

    printf("loading fonts...\n");
    initFonts();
    printf("loading textures...\n");
    initTexture(d);

    // Initialize OpenGL settings
    initGLGame();

    // Set up callbacks
    glutDisplayFunc(displayGame);
    glutReshapeFunc(onReshape);
    glutKeyboardFunc(keyGame);
    glutSpecialFunc(specialGame);
    glutIdleFunc(idleGame);
#endif
}

int main( int argc, char *argv[] ) {
    char *path;

#ifdef __FreeBSD__
    fpsetmask(0);
#endif

#ifndef ANDROID
    glutInit(&argc, argv);
#endif

    // Print current working directory
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working directory: %s\n", cwd);
    }

    path = getFullPath("settings.txt");
    if(path != 0)
        initMainGameSettings(path); /* reads defaults from ~/.gltronrc */
    else {
        printf("fatal: could not settings.txt, exiting...\n");
        exit(1);
    }

    parse_args(argc, argv);

    /* sound */

#ifdef SOUND
    printf("initializing sound\n");
    initSound();

    // Print sound file search paths
    printf("Sound file search paths:\n");
    printf("1. Current directory: %s\n", cwd);
    printf("2. /usr/share/games/gltron/\n");
    printf("3. /usr/local/share/games/gltron/\n");

    path = getFullPath("gltron.it");
    if(path == 0 || loadSound(path))
        printf("error trying to load sound\n");
    else {
        if(game->settings->playSound) {
            playSound();
            free(path);
        }
    }
#endif

    printf("loading menu\n");
    path = getFullPath("menu.txt");
    if(path != 0)
        pMenuList = loadMenuFile(path);
    else {
        printf("fatal: could not load menu.txt, exiting...\n");
        exit(1);
    }
    printf("menu loaded\n");
    free(path);

    initGameStructures();
    resetScores();

    initData();

    setupDisplay(game->screen);
    switchCallbacks(&guiCallbacks);
    switchCallbacks(&guiCallbacks);

#ifndef ANDROID
    glutMainLoop();
#else
    /* Android: the app's activity should drive the main loop */
#endif

    return 0;
}

callbacks gameCallbacks = { 
  displayGame,
  idleGame,
  keyGame,
  specialGame,
  initGame,
  initGLGame
};






