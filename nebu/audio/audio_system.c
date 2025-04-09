#include "audio/nebu_audio_system.h"

#include "base/nebu_system.h"
#include "base/sdl_compat.h"
#include <stdio.h>

void nebu_Audio_Init(void)
{
	if(SDL_Init_Compat(SDL_INIT_AUDIO) < 0 ){
		fprintf(stderr, "Couldn't initialize SDL audio: %s\n", SDL_GetError_Compat());
		/* FIXME: disable sound system */
	}
}

