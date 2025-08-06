#include "video/video.h"
#include "game/game.h"
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

int initWindow(void) {
    int win_id;
    int flags;

    printf("[init] Initializing window\n");

    /* Use default values instead of reading from Lua */
    int width = 800;
    int height = 600;
    int bitdepth_32 = 1;
    int windowMode = 1;  // Changed to 1 for windowed mode
    int use_stencil = 1;
    int mouse_warp = 1;

    printf("[init] Setting window mode: %dx%d\n", width, height);
    nebu_Video_SetWindowMode(0, 0, width, height);

    flags = SYSTEM_RGBA | SYSTEM_DOUBLE | SYSTEM_DEPTH | SYSTEM_STENCIL;
    if(bitdepth_32)
        flags |= SYSTEM_32_BIT;
    if(windowMode == 0)  // This is now false
        flags |= SYSTEM_FULLSCREEN;

    printf("[init] Setting display mode: flags=%d\n", flags);
    nebu_Video_SetDisplayMode(flags);

    printf("[init] Creating window\n");
    win_id = nebu_Video_Create("gltron");
    printf("[init] Window created: win_id=%d\n", win_id);

    // Check OpenGL initialization
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* version = (const char*)glGetString(GL_VERSION);

    printf("[opengl] Vendor: %s\n", vendor ? vendor : "Unknown");
    printf("[opengl] Renderer: %s\n", renderer ? renderer : "Unknown");
    printf("[opengl] Version: %s\n", version ? version : "Unknown");

    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("[opengl] Error initializing OpenGL: %d\n", error);
    } else {
        printf("[opengl] OpenGL initialized successfully\n");
    }

    // check if we have destination alpha,
    // if not, display warning
    {
        int r, g, b, a;
        nebu_Video_GetDisplayDepth(&r, &g, &b, &a);
        printf("[init] Display depth: r=%d, g=%d, b=%d, a=%d\n", r, g, b, a);
        if(a == 0) {
            /* Skip executing Lua code */
            printf("[warning] No destination alpha available\n");
        }
    }

    if (win_id < 0) {
        if(use_stencil) {
            flags &= ~SYSTEM_STENCIL;
            printf("[init] Retrying without stencil: flags=%d\n", flags);
            nebu_Video_SetDisplayMode(flags);
            win_id = nebu_Video_Create("gltron");
            printf("[init] Window created: win_id=%d\n", win_id);
            if(win_id >= 0) {
                /* Skip updating setting in Lua */
                use_stencil = 0;
                goto SKIP;
            }
        }
        printf("[fatal] could not create window...exiting\n");
        nebu_assert(0); exit(1); /* OK: critical, no visual */
    }

    SKIP:

    if(windowMode == 0 || mouse_warp == 1) {
        printf("[init] Grabbing input\n");
        nebu_Input_Grab();
    } else {
        printf("[init] Not grabbing input\n");
        nebu_Input_Ungrab();
    }

    printf("[init] Window initialized with default values\n");
    return win_id;
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
    gScreen->win_id = initWindow();
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

    // Call our simplified video_CreateLevel function
    gWorld = video_CreateLevel();
    if (!gWorld) {
        fprintf(stderr, "[FATAL] Failed to create world\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the skybox
    gWorld->Skybox = (Skybox*)malloc(sizeof(Skybox));
    if (!gWorld->Skybox) {
        fprintf(stderr, "[FATAL] Failed to allocate memory for skybox\n");
        exit(EXIT_FAILURE);
    }

    // Load skybox textures
    const char* skyboxFilenames[6] = {
        "art/classic/skybox0.png", //front
        "art/classic/skybox2.png", //back
        "art/classic/skybox3.png", //left
        "art/classic/skybox4.png", //right
        "art/classic/skybox5.png", //top
        "art/classic/skybox1.png" //bottom
    };
    loadSkyboxTextures(gWorld->Skybox, skyboxFilenames);

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
