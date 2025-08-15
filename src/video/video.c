#include "video/video.h"
#include "video/shader_manager.h"
#include "game/game.h"
#include "video/skybox.h"
#include "game/resource.h"
#include "base/nebu_resource.h"
#include "filesystem/path.h"
#include "configuration/settings.h"

#include "Nebu_video.h"
#include "Nebu_input.h"
#include "Nebu_scripting.h"

#include <string.h>
#include <png.h>

#include "base/nebu_debug_memory.h"

#include "base/nebu_assert.h"

#include "video/skybox.h"
#include <SDL2/SDL.h>

#ifdef __ANDROID__
  #include <GLES2/gl2.h>
#else
  #include <GL/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>

// Global variables
int gScreenWidth = 800;
int gScreenHeight = 600;
SDL_Window *gWindow = NULL;
SDL_GLContext gGLContext = NULL;

// Platform detection
#ifdef __ANDROID__
#define IS_ANDROID 1
#else
#define IS_ANDROID 0
#endif

// Utility: Print OpenGL info
static void printGLInfo() {
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);
    printf("[opengl] Vendor: %s\n", vendor ? vendor : "Unknown");
    printf("[opengl] Renderer: %s\n", renderer ? renderer : "Unknown");
    printf("[opengl] Version: %s\n", version ? version : "Unknown");
}

// Utility: Set Android immersive mode (hides nav/status bars)
static void setAndroidImmersiveMode() {
#if IS_ANDROID
    // For Android, we need to set the window flags to immersive mode
    // This is typically done when creating the window
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");

    // For immersive mode, we need to set the window flags when creating the window
    // The actual immersive mode is handled by the system UI flags
    // We'll set these flags when creating the window in initWindow()
#endif
}

// Example usage in your setupDisplay or main
void setupDisplayCrossPlatform() {
    int width = 800;
    int height = 600;
    int fullscreen = 0;
#if IS_ANDROID
    fullscreen = 1; // Always fullscreen on Android
#else
    // Read from config or user settings for PC
    // fullscreen = getSettingi("fullscreen");
#endif
    initWindow(width, height, fullscreen);
}

void shutdownDisplayCrossPlatform() {
    if (gGLContext) {
        SDL_GL_DeleteContext(gGLContext);
        gGLContext = NULL;
    }
    if (gWindow) {
        SDL_DestroyWindow(gWindow);
        gWindow = NULL;
    }
    SDL_Quit();
}

void displayGame(void) {
    printf("[display] Drawing game\n");
    drawGame();
    printf("[display] Swapping buffers\n");
    nebu_System_SwapBuffers();
    printf("[display] Game displayed\n");
}

void video_LoadResources(void)
{
}

void video_ReleaseResources(void)
{
}

// In your initialization function
void initShaders() {
    printf("Initializing all shaders...\n");

    // Initialize basic shaders
    initBasicShader();

    // Initialize skybox shaders
    initSkyboxShaders();

    if (!gShaderProgram || !gSkyboxShaderProgram) {
        fprintf(stderr, "Failed to initialize shaders\n");
        exit(1);
    }

    printf("All shaders initialized successfully\n");
}

// In your rendering function
void renderScene() {
    // Use the shader program
    glUseProgram(gShaderProgram);

    // Set the MVP matrix uniform
    float mvp[16];
    // Set up your MVP matrix here (identity matrix for simplicity)
    for (int i = 0; i < 16; i++) {
        mvp[i] = (i % 5 == 0) ? 1.0f : 0.0f; // Simple identity matrix
    }
    glUniformMatrix4fv(gUniformMVP, 1, GL_FALSE, mvp);

    // Enable the position attribute
    glEnableVertexAttribArray(gAttribPosition);

    // Set up your vertex data and draw calls here
    // ...

    // Disable the attribute when done
    glDisableVertexAttribArray(gAttribPosition);

    // Unbind the shader program when done
    glUseProgram(0);
}

// Main window/context initialization
int initWindow(int width, int height, int fullscreen) {
    printf("[init] Initializing window: %dx%d fullscreen=%d\n", width, height, fullscreen);

    // SDL2 video init
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "[fatal] SDL_Init failed: %s\n", SDL_GetError());
        exit(1);
    }

    Uint32 winFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

#if IS_ANDROID
    // On Android, we want fullscreen immersive mode
    winFlags |= SDL_WINDOW_FULLSCREEN;

    // Set hints for Android immersive mode
    SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");

    // Create window with immersive mode flags
    gWindow = SDL_CreateWindow("gltron",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               width, height,
                               winFlags);

    // Set immersive mode flags after window creation
    if (gWindow) {
        SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
        // For immersive mode, we need to set system UI flags
        // This is typically done through native Android code
    }
#else
    if (fullscreen)
        winFlags |= SDL_WINDOW_FULLSCREEN;
    gWindow = SDL_CreateWindow("gltron",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               width, height,
                               winFlags);
#endif

    if (!gWindow) {
        fprintf(stderr, "[fatal] SDL_CreateWindow failed: %s\n", SDL_GetError());
        exit(1);
    }

    // OpenGL context attributes
#if IS_ANDROID
    // OpenGL ES 2.0 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    // Desktop OpenGL 2.1 context (or higher if you want)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

    gGLContext = SDL_GL_CreateContext(gWindow);
    if (!gGLContext) {
        fprintf(stderr, "[fatal] SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        exit(1);
    }

    SDL_GL_MakeCurrent(gWindow, gGLContext);

    // Vsync (optional)
    SDL_GL_SetSwapInterval(1);

    // Print OpenGL info
    printGLInfo();

    // Verify OpenGL context
    GLint contextFlags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &contextFlags);
    printf("OpenGL context flags: 0x%X\n", contextFlags);

    // Check OpenGL version
    const char* version = (const char*)glGetString(GL_VERSION);
    printf("OpenGL version: %s\n", version ? version : "Unknown");

    // Set viewport
    glViewport(0, 0, width, height);

    // Initialize shaders after OpenGL context is created
    printf("Initializing shaders...\n");
    initShaders();
    printf("Shaders initialized\n");

    printf("[init] Window and OpenGL context initialized successfully\n");
    return 0;
}

void reshape(int x, int y) {
	if(x < getSettingi("height") || x < getSettingi("width"))
		initGameScreen();
	if(x > getSettingi("width") )
		gScreen->vp_x = (x - getSettingi("width")) / 2;
	if(y > getSettingi("height") )
		gScreen->vp_y = (y - getSettingi("height")) / 2;
	if(game)
		changeDisplay(-1);
}

void shutdownDisplay() {
	resource_ReleaseType(eRT_Font);
	resource_ReleaseType(eRT_Texture);
	resource_ReleaseType(eRT_2d);
	gui_ReleaseResources();
	nebu_Video_Destroy(gScreen->win_id);
	// printf("[video] window destroyed\n");
}

void setupDisplay(void) {
    printf("[video] Setting up display\n");

    /* Get display settings from configuration */
    int width = getSettingi("width");
    int height = getSettingi("height");
    int fullscreen = !getSettingi("windowMode");  /* windowMode=1 means windowed, so fullscreen is opposite */
    int flags = 0;

    printf("[video] Using settings: width=%d, height=%d, fullscreen=%d\n", width, height, fullscreen);

    /* Set window mode first */
    nebu_Video_SetWindowMode(0, 0, width, height);

    /* Then set display mode flags */
    flags = SYSTEM_RGBA | SYSTEM_DOUBLE | SYSTEM_DEPTH | SYSTEM_STENCIL;
    if (fullscreen)
        flags |= SYSTEM_FULLSCREEN;

    /* Call nebu_Video_SetDisplayMode with the correct arguments */
    nebu_Video_SetDisplayMode(flags);
    printf("[video] Display mode set successfully\n");

    /* Create the window */
    printf("[video] Creating window\n");
    gScreen->win_id = initWindow(width, height, fullscreen);
    printf("[video] Window created with ID: %d\n", gScreen->win_id);

    printf("[video] Display setup complete\n");
}

void loadModels(void)
{
	char *path;
	int i;

	nebu_assert(!gTokenRecognizer);
	nebu_assert(!gTokenRecognizerQuad);

	for(i = 0; i < LC_LOD; i++)
	{
		nebu_assert(!gpTokenLightcycles[i]);
	}

	/* load recognizer model */
	path = getPath(PATH_DATA, "recognizer.obj");
	gTokenRecognizer = resource_GetToken(path, eRT_GLtronTriMesh);
	if(!gTokenRecognizer)
	{
		fprintf(stderr, "fatal: could not load %s - exiting...\n", path);
		nebu_assert(0); exit(1); // OK: critical, installation corrupt
	}
	free(path);

	/* load recognizer quad model (for recognizer outlines) */
	path = getPath(PATH_DATA, "recognizer_quad.obj");
	gTokenRecognizerQuad = resource_GetToken(path, eRT_GLtronQuadMesh);
	if(!gTokenRecognizerQuad)
	{
		fprintf(stderr, "fatal: could not load %s - exiting...\n", path);
		nebu_assert(0); exit(1); // OK: critical, installation corrupt
	}
	free(path);

	/* load lightcycle models */
	for(i = 0; i < LC_LOD; i++) {
		path = getPath(PATH_DATA, lc_lod_names[i]);
		gpTokenLightcycles[i] = resource_GetToken(path, eRT_GLtronTriMesh);
		if(!gpTokenLightcycles[i])
		{
			fprintf(stderr, "fatal: could not load model %s - exiting...\n", lc_lod_names[i]);
			nebu_assert(0); exit(1); // OK: critical, installation corrupt
		}
		free(path);
	}
}

void freeVideoData(void)
{
	int i;
	for(i = 0; i < gnPlayerVisuals; i++)
	{
		free(gppPlayerVisuals[i]);
	}
	free(gppPlayerVisuals);
	free(gScreen->textures);
	free(gScreen->ridTextures);
	free(gScreen);
}

void initVideoData(void) {
    int i;

    printf("[init] Initializing video data\n");

    gScreen = (Visual*) malloc(sizeof(Visual));
    if (!gScreen) {
        fprintf(stderr, "[error] Failed to allocate memory for gScreen\n");
        return;
    }

    memset(gScreen, 0, sizeof(Visual));

    gViewportType = getSettingi("display_type");

    {
        Visual *d = gScreen;
        d->w = getSettingi("width");
        d->h = getSettingi("height");
        d->vp_x = 0;
        d->vp_y = 0;
        d->vp_w = d->w;
        d->vp_h = d->h;
        d->onScreen = -1;
        d->ridTextures = (int*) malloc(TEX_COUNT * sizeof(int));
        if (!d->ridTextures) {
            fprintf(stderr, "[error] Failed to allocate memory for ridTextures\n");
            return;
        }
        memset(d->ridTextures, 0, TEX_COUNT * sizeof(int));
        d->textures = (unsigned int*) malloc(TEX_COUNT * sizeof(unsigned int));
        if (!d->textures) {
            fprintf(stderr, "[error] Failed to allocate memory for textures\n");
            return;
        }
        memset(d->textures, 0, TEX_COUNT * sizeof(unsigned int));
    }

    gnPlayerVisuals = MAX_PLAYER_VISUALS;
    gppPlayerVisuals = (PlayerVisual**) malloc(gnPlayerVisuals * sizeof(PlayerVisual*));
    if (!gppPlayerVisuals) {
        fprintf(stderr, "[error] Failed to allocate memory for gppPlayerVisuals\n");
        return;
    }

    for(i = 0; i < gnPlayerVisuals; i++)
    {
        gppPlayerVisuals[i] = (PlayerVisual*) malloc(sizeof(PlayerVisual));
        if (!gppPlayerVisuals[i]) {
            fprintf(stderr, "[error] Failed to allocate memory for gppPlayerVisuals[%d]\n", i);
            return;
        }
        memset(gppPlayerVisuals[i], 0, sizeof(PlayerVisual));
        gppPlayerVisuals[i]->pPlayer = NULL;
    }

    for(i = 0; i < eHUDElementCount; i++)
    {
        gpTokenHUD[i] = 0;
    }

    printf("[init] Video data initialized successfully\n");
}

void initGameScreen(void)
{
	Visual *d;
	d = gScreen;
	d->w = getSettingi("width");
	d->h = getSettingi("height");
	d->vp_x = 0; d->vp_y = 0;
	d->vp_w = d->w; d->vp_h = d->h;
}

void video_UnloadLevel(void)
{
	if(gWorld)
		video_FreeLevel(gWorld);
	gWorld = NULL;
}

void video_LoadLevel(void) {
    printf("[video] Loading level\n");

    if (gWorld) {
        printf("[video] Freeing existing world\n");
        video_FreeLevel(gWorld);
        gWorld = NULL;
    } else {
        printf("[video] No existing world to free\n");
    }

    printf("[video] Creating new world\n");

    // Allocate memory for the world
    gWorld = (video_level*)malloc(sizeof(video_level));
    if (!gWorld) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for world\n");
        return;
    }

    // Initialize the world structure
    memset(gWorld, 0, sizeof(video_level));

    // Initialize the skybox
    gWorld->Skybox = (Skybox*)malloc(sizeof(Skybox));
    if (!gWorld->Skybox) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for skybox\n");
        free(gWorld);
        gWorld = NULL;
        return;
    }

    // Initialize the skybox with proper shader setup
    Skybox* skybox = gWorld->Skybox;
    initModernSkybox(skybox);

    // Set up skybox shaders
    skybox->shaderProg = gSkyboxShaderProgram;
    skybox->uMVP = gSkyboxUniformMVP;
    skybox->uSkybox = gSkyboxUniformSkybox;
    skybox->aPosition = gSkyboxAttribPosition;

    // Load skybox textures using the TextureInfo array
    for (int i = 0; i < TEX_COUNT; i++) {
        if (textures[i].type == TEX_SKYBOX0 || textures[i].type == TEX_SKYBOX1 ||
            textures[i].type == TEX_SKYBOX2 || textures[i].type == TEX_SKYBOX3 ||
            textures[i].type == TEX_SKYBOX4 || textures[i].type == TEX_SKYBOX5) {

            int textureId = resource_LoadTexture(textures[i].filename, textures[i].texture_type);
            if (textureId < 0) {
                fprintf(stderr, "[ERROR] Failed to load skybox texture: %s\n", textures[i].filename);
                free(gWorld->Skybox);
                free(gWorld);
                gWorld = NULL;
                return;
            }

            // Assign the texture to the appropriate skybox face
            switch (textures[i].type) {
                case TEX_SKYBOX0: skybox->textures[0] = textureId; break;
                case TEX_SKYBOX1: skybox->textures[1] = textureId; break;
                case TEX_SKYBOX2: skybox->textures[2] = textureId; break;
                case TEX_SKYBOX3: skybox->textures[3] = textureId; break;
                case TEX_SKYBOX4: skybox->textures[4] = textureId; break;
                case TEX_SKYBOX5: skybox->textures[5] = textureId; break;
            }
        }
    }

    printf("[video] World created successfully\n");
}

void video_ResetData(void) {
    int i;

    printf("[status] reset video data (stub)\n");

    /* Use default values instead of reading from Lua */
    float defaultDiffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float defaultSpecular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float defaultAlpha[4] = {1.0f, 1.0f, 1.0f, 0.8f};

    for(i = 0; i < game->players; i++) {
        Player *p = game->player + i;
        {
            /* Skip reading from Lua, use default values */
            memcpy(p->profile.pColorDiffuse, defaultDiffuse, 4 * sizeof(float));
            memcpy(p->profile.pColorSpecular, defaultSpecular, 4 * sizeof(float));
            memcpy(p->profile.pColorAlpha, defaultAlpha, 4 * sizeof(float));
        }
    }

    printf("[status] video data reset with default values\n");
}

void initDisplay(Visual *d, int type, int p, int onScreen) {
	float field_x = gScreen->vp_w / 32.0f;
	float field_y = gScreen->vp_h / 24.0f;
	d->h = gScreen->h;
	d->w = gScreen->w;
	d->vp_x = gScreen->vp_x + (int) ( vp_x[type][p] * field_x );
	d->vp_y = gScreen->vp_y + (int) ( vp_y[type][p] * field_y );
	d->vp_w = (int) ( vp_w[type][p] * field_x );
	d->vp_h = (int) ( vp_h[type][p] * field_y );
	d->onScreen = onScreen;
}

static void defaultViewportPositions(void) {
	int i;

	for(i = 0; i < gnPlayerVisuals; i++)
	{
		gppPlayerVisuals[i]->pPlayer = &game->player[i % game->players];
	}
}

/*
  autoConfigureDisplay - configure viewports so every human player has one
 */
static void autoConfigureDisplay(void) {
	int n_humans = 0;
	int i;
	int vp;

	// nebu_assert(gnPlayerVisuals <= game->players);
	// the other way round makes more sense
	nebu_assert(getSettingi("players") <= gnPlayerVisuals);

	defaultViewportPositions();

	/* loop thru players and find the humans */
	for (i=0; i < game->players; i++) {
		if (game->player[i].ai.active == AI_HUMAN)
		{
			gppPlayerVisuals[n_humans]->pPlayer = &game->player[i];
			n_humans++;
		}
	}

	switch(n_humans) {
	case 0 :
		/*
			Not sure what the default should be for
			a game without human players. For now 
			just show a single viewport.
		*/
		/* fall thru */
	case 1 :
		vp = VP_SINGLE;
		break;
	case 2 :
		vp = VP_SPLIT;
		break;
	default :
		defaultViewportPositions();
		vp = VP_FOURWAY;
	}
	updateDisplay(vp);
}

void changeDisplay(int view) {
	nebu_assert(game);

	/* passing -1 to changeDisplay tells it to use the view from settings */
	if (view == -1) {
		view = getSettingi("display_type");
	}

	if (view == 3) {
		autoConfigureDisplay();
	} else {
		defaultViewportPositions();
		updateDisplay(view);
	}

	// displayMessage(TO_STDOUT, "set display to %d", view);
	setSettingi("display_type", view);
}

void updateDisplay(int vpType) {
	int i;

	gViewportType = vpType;

	for (i = 0; i < gnPlayerVisuals; i++) {
		gppPlayerVisuals[i]->display.onScreen = 0;
	}
	for (i = 0; i < vp_max[vpType]; i++) {
		initDisplay(& gppPlayerVisuals[i]->display, vpType, i, 1);
	}

}

void Video_Idle(void) { }
