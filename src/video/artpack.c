#include "video/video.h"
#include "game/game.h"
#include "game/resource.h"
#include "base/nebu_resource.h"
#include "filesystem/path.h"
#include "base/util.h"
#include "video/video_level.h"

#include "Nebu_scripting.h"
#include "Nebu_filesystem.h"

#include <string.h>
#include "base/nebu_assert.h"

#include <base/nebu_debug_memory.h>

/*!
	scan the artpack's directory's ( art/ ) contents and store the
	file/directory names in a lua list
*/

void initArtpacks(void) {
    printf("[init] Initializing artpacks (stub)\n");
    
    const char *art_path;
    nebu_List *artList;
    nebu_List *p;
    
    art_path = getDirectory(PATH_ART);
    artList = readDirectoryContents(art_path, NULL);
    if(artList->next == NULL) {
        fprintf(stderr, "[fatal] no art files found...exiting\n");
        nebu_assert(0); exit(1); // OK: critical, installation corrupt
    }
    
    /* Skip creating Lua table and calling Lua functions */
    printf("[init] Found artpacks:\n");
    for(p = artList; p->next != NULL; p = p->next) {
        printf("[init] - %s\n", (char*) p->data);
        free(p->data);
    }
    nebu_List_Free(artList);
    
    /* Set default artpack - use the correct field name */
    /* If there's no artpack field, you can remove this line */
    /* Or you can set a different field that does exist */
    
    printf("[init] Artpacks initialized with default values\n");
}

/*! load the HUD surfaces */
void artpack_LoadSurfaces(void)
{
	// char *pHUDNames[eHUDElementCount] = {
	char *pHUDNames[] = {
		"hud-speed.png",
		"hud-mask-speed.png",
		"hud-mask-turbo.png",
		"hud-ai.png",
		"hud-map.png",
		"hud-scores.png",
		"hud-fps.png",
		"hud-buster.png",
		"hud-mask-buster.png"
	};
	int i;
	for(i = 0; i < eHUDElementCount; i++)
	{
		nebu_assert(!gpTokenHUD[i]);

		gpTokenHUD[i] = resource_GetToken(pHUDNames[i], eRT_2d);
		if(!gpTokenHUD[i])
		{
			fprintf(stderr, "fatal: failed loading %s, exiting...\n", pHUDNames[i]);
			nebu_assert(0); exit(1); // OK: critical, installation corrupt
		}
	}
}

/*!
	parse global & customized artpack.lua files
	
*/
void loadArt(void) {
    printf("[init] Loading art (stub)\n");
    
    /* Skip loading Lua scripts */
    printf("[init] Skipping artpack.lua script loading\n");
    
    /* Load textures and fonts directly */
    initTexture(gScreen); // load skybox, trail & crash texture
    fprintf(stderr, "[status] done loading textures...\n");
    initFonts();
    fprintf(stderr, "[status] done loading fonts...\n");
    
    artpack_LoadSurfaces();
}
