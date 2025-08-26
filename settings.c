#include "gltron.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 100

#ifdef ANDROID
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "GLTron", __VA_ARGS__)
#endif

void initSettingData(char *filename) {
#ifdef ANDROID
  LOGI("initSettingData: begin");
#endif
  FILE *f = NULL;
  int n = 0, i, count = 0, j;
  char buf[BUFSIZE];
  char c = 0;

  if (!filename || filename[0] == '\0') {
#ifdef ANDROID
    LOGI("initSettingData: invalid filename");
#endif
    return;
  }

  // Try to resolve via getFullPath first (desktop may package settings in share dir)
#ifdef ANDROID
  // On Android, settings is in internal storage path; do not use getFullPath (assets are read-only)
  char *resolved = NULL;
  const char *openName = filename;
  LOGI("initSettingData: resolved path: %s", openName ? openName : "(null)");
#else
  char *resolved = getFullPath(filename);
  const char *openName = resolved ? resolved : filename;
#endif

  f = fopen(openName, "r");
  if (!f) {
#ifdef ANDROID
    LOGI("initSettingData: fopen failed for %s", openName ? openName : "(null)");
#endif
    if (resolved) free(resolved);
    return; // fall back to defaults
  }

  if (!fgets(buf, BUFSIZE, f)) {
    fclose(f);
    if (resolved) free(resolved);
    return;
  }
  if (sscanf(buf, "%d ", &n) != 1 || n < 0 || n > 1024) {
    fclose(f);
    if (resolved) free(resolved);
    return;
  }
  for(i = 0; i < n; i++) {
    if (!fgets(buf, BUFSIZE, f)) break;
    if (sscanf(buf, "%c%d ", &c, &count) != 2 || count < 0 || count > 1024) {
      continue;
    }
    switch(c) {
    case 'i': /* it's int */
      free(si); si = NULL; si_count = 0;
      si = malloc(sizeof(struct settings_int) * (size_t)count);
      if (!si) { fclose(f); if (resolved) free(resolved); return; }
      si_count = count;
      for(j = 0; j < count; j++) {
        if (!fgets(buf, BUFSIZE, f)) { si_count = j; break; }
        buf[BUFSIZE - 1] = 0;
        // Read a token into name (max 31 chars + NUL)
        if (sscanf(buf, "%31s", (si + j)->name) != 1) {
          (si + j)->name[0] = '\0';
        }
      }
#ifdef ANDROID
      LOGI("initSettingData: parsed %d integer settings", si_count);
#endif
      break;
    case 'f': /* float */
      free(sf); sf = NULL; sf_count = 0;
      sf = malloc(sizeof(struct settings_float) * (size_t)count);
      if (!sf) { fclose(f); if (resolved) free(resolved); return; }
      sf_count = count;
      for(j = 0; j < count; j++) {
        if (!fgets(buf, BUFSIZE, f)) { sf_count = j; break; }
        buf[BUFSIZE - 1] = 0;
        if (sscanf(buf, "%31s", (sf + j)->name) != 1) {
          (sf + j)->name[0] = '\0';
        }
      }
#ifdef ANDROID
      LOGI("initSettingData: parsed %d float settings", sf_count);
#endif
      break;
    default:
#ifdef ANDROID
      LOGI("initSettingData: unrecognized type '%c'", c);
#endif
      // ignore unknown sections instead of aborting
      break;
    }
  }

  if (resolved) free(resolved);

  if (!game || !game->settings) return;

  // Ensure arrays are allocated to expected minimal sizes to avoid later deref
  if (!si || si_count < 28) {
    if (si) free(si);
    si = calloc(28, sizeof(struct settings_int));
    si_count = 28;
    // Initialize names to match defaults if parsing failed
    const char* names_int[28] = {
      "show_help","show_fps","show_wall","show_glow","show_2d","show_alpha","show_floor_texture","line_spacing","erase_crashed","fast_finish","fov","width","height","show_ai_status","camType","display_type","playSound","show_model","ai_player1","ai_player2","ai_player3","ai_player4","show_crash_texture","turn_cycle","mouse_warp","sound_driver","input_mode","fullscreen"
    };
    for (int k = 0; k < 28; ++k) strncpy(si[k].name, names_int[k], sizeof(si[k].name)-1);
  }
  if (!sf || sf_count < 1) {
    if (sf) free(sf);
    sf = calloc(1, sizeof(struct settings_float));
    sf_count = 1;
    strncpy(sf[0].name, "speed", sizeof(sf[0].name)-1);
  }

  // Bind pointers
  si[0].value = &(game->settings->show_help);
  si[1].value = &(game->settings->show_fps);
  si[2].value = &(game->settings->show_wall);
  si[3].value = &(game->settings->show_glow);
  si[4].value = &(game->settings->show_2d);
  si[5].value = &(game->settings->show_alpha);
  si[6].value = &(game->settings->show_floor_texture);
  si[7].value = &(game->settings->line_spacing);
  si[8].value = &(game->settings->erase_crashed);
  si[9].value = &(game->settings->fast_finish);
  si[10].value = &(game->settings->fov);
  si[11].value = &(game->settings->width);
  si[12].value = &(game->settings->height);
  si[13].value = &(game->settings->show_ai_status);
  si[14].value = &(game->settings->camType);
  si[15].value = &(game->settings->display_type);
  si[16].value = &(game->settings->playSound);
  si[17].value = &(game->settings->show_model);
  si[18].value = &(game->settings->ai_player1);
  si[19].value = &(game->settings->ai_player2);
  si[20].value = &(game->settings->ai_player3);
  si[21].value = &(game->settings->ai_player4);
  si[22].value = &(game->settings->show_crash_texture);
  si[23].value = &(game->settings->turn_cycle);
  si[24].value = &(game->settings->mouse_warp);
  si[25].value = &(game->settings->sound_driver);
  /* input_mode appended at the end of ints */
  if (si_count > 26) {
    si[26].value = &(game->settings->input_mode);
  }
  /* fullscreen appended after input_mode */
  if (si_count > 27) {
    si[27].value = &(game->settings->fullscreen);
  }

  sf[0].value = &(game->settings->speed);
}

int* getVi(char* name) {
  int i;
  for(i = 0; i < si_count; i++) {
    if(strstr(name, si[i].name) == name) 
      return si[i].value;
  }
  return 0;
}

void initMainGameSettings(char *filename) {
  char *fname, *home;
  char buf[100];
  char expbuf[100];
  int i;
  FILE* f;

  game = &main_game;
  game->settings = (Settings*) malloc(sizeof(Settings));
  if (!game->settings) return;
  memset(game->settings, 0, sizeof(Settings));

  /* initialize defaults first, then load names/bindings from file */
  game->pauseflag = 0;

  game->settings->show_help = 0;
  game->settings->show_fps = 1;
  game->settings->show_wall = 1;
  game->settings->show_glow = 1;
  game->settings->show_2d = 0;
  game->settings->show_alpha = 1;
  game->settings->show_floor_texture = 1;
  game->settings->show_crash_texture = 1;
  game->settings->show_model = 1;
  game->settings->turn_cycle = 1;
  game->settings->line_spacing = 20;
  game->settings->erase_crashed = 0;
  game->settings->fast_finish = 1;
  game->settings->fov = 105;
  game->settings->speed = 4.2;
  game->settings->width = 640;
  game->settings->height = 480;
  game->settings->show_ai_status = 1;
  game->settings->camType = 0;
  game->settings->mouse_warp = 0;
  game->settings->input_mode = 1; /* default to Mouse for menu usability */
#ifdef ANDROID
  game->settings->fullscreen = 1;
#else
  game->settings->fullscreen = 0;
#endif
  game->settings->display_type = 0;
  game->settings->playSound = 1;
  game->settings->playMusic = 1;

  game->settings->ai_player1 = 0;
  game->settings->ai_player2 = 1;
  game->settings->ai_player3 = 1;
  game->settings->ai_player4 = 1;

  game->settings->sound_driver = 0;

  /* not included in .gltronrc */

  game->settings->screenSaver = 0;
  game->settings->windowMode = 0;
  game->settings->content[0] = 0;
  game->settings->content[1] = 1;
  game->settings->content[2] = 2;
  game->settings->content[3] = 3;

  /* now load settings names and bind pointers */
  initSettingData(filename);

  game->settings->show_help = 0;
  game->settings->show_fps = 1;
  game->settings->show_wall = 1;
  game->settings->show_glow = 1;
  game->settings->show_2d = 0;
  game->settings->show_alpha = 1;
  game->settings->show_floor_texture = 1;
  game->settings->show_crash_texture = 1;
  game->settings->show_model = 1;
  game->settings->turn_cycle = 1;
  game->settings->line_spacing = 20;
  game->settings->erase_crashed = 0;
  game->settings->fast_finish = 1;
  game->settings->fov = 105;
  game->settings->speed = 4.2;
  game->settings->width = 640;
  game->settings->height = 480;
  game->settings->show_ai_status = 1;
  game->settings->camType = 0;
  game->settings->mouse_warp = 0;
  game->settings->input_mode = 1; /* default to Mouse for menu usability */
#ifdef ANDROID
  game->settings->fullscreen = 1;
#else
  game->settings->fullscreen = 0;
#endif

  game->settings->display_type = 0;
  game->settings->playSound = 1;
  game->settings->playMusic = 1;

  game->settings->ai_player1 = 0;
  game->settings->ai_player2 = 1;
  game->settings->ai_player3 = 1;
  game->settings->ai_player4 = 1;
  
  game->settings->sound_driver = 0;

  /* not included in .gltronrc */

  game->settings->screenSaver = 0;
  game->settings->windowMode = 0;
  game->settings->content[0] = 0;
  game->settings->content[1] = 1;
  game->settings->content[2] = 2;
  game->settings->content[3] = 3;

  /* go for .gltronrc (or whatever is defined in RC_NAME) */

#ifndef ANDROID
  home = getenv(HOMEVAR);
  if(home == 0) /* evaluate homedir */ {
    fname = malloc(strlen(CURRENT_DIR) + strlen(RC_NAME) + 2);
    sprintf(fname, "%s%c%s", CURRENT_DIR, SEPERATOR, RC_NAME);
  }
  else {
    fname = malloc(strlen(home) + strlen(RC_NAME) + 2);
    sprintf(fname, "%s%c%s", home, SEPERATOR, RC_NAME);
  }
  f = fopen(fname, "r");
  if(f == 0) {
    printf("no %s found - using defaults\n", fname);
    free(fname);
    return; /* no rc exists */
  }
  while(fgets(buf, sizeof(buf), f)) {
    /* process rc-file */

    if(strstr(buf, "iset") == buf) {
      /* linear search through settings */
      /* first: integer */
      for(i = 0; i < si_count; i++) {
	sprintf(expbuf, "iset %s ", si[i].name);
	if(strstr(buf, expbuf) == buf) {
	  sscanf(buf + strlen(expbuf), "%d ", si[i].value);
	  printf("assignment: %s\t%d\n", si[i].name, *(si[i].value));
	  break;
	}
      }
    } else if(strstr(buf, "fset") == buf) {
      for(i = 0; i < sf_count; i++) {
	sprintf(expbuf, "fset %s ", sf[i].name);
	if(strstr(buf, expbuf) == buf) {
	  sscanf(buf + strlen(expbuf), "%f ", sf[i].value);
	  printf("assignment: %s\t%.2f\n", sf[i].name, *(sf[i].value));
	  break;
	}
      }
    }
  }
  free(fname);
  fclose(f);
#else
  /* On Android, skip reading rc file to avoid early aborts; defaults + parsed settings are sufficient */
  (void)fname; (void)home; (void)i; (void)buf; (void)expbuf; (void)f;
#endif
#ifdef ANDROID
  // Always enforce fullscreen on Android at load time (surface applies real size)
  game->settings->fullscreen = 1;
#endif

}

void saveSettings() {
  char *fname, *home;
  int i;
  FILE* f;

#ifndef ANDROID
  home = getenv(HOMEVAR);
  if(home == 0) /* evaluate homedir */ {
    fname = malloc(strlen(CURRENT_DIR) + strlen(RC_NAME) + 2);
    sprintf(fname, "%s%c%s", CURRENT_DIR, SEPERATOR, RC_NAME);
  }
  else {
    fname = malloc(strlen(home) + strlen(RC_NAME) + 2);
    sprintf(fname, "%s%c%s", home, SEPERATOR, RC_NAME);
  }
#else
  extern char s_base_path[1024];
  fname = malloc(strlen(s_base_path) + 1 + strlen(RC_NAME) + 1);
  sprintf(fname, "%s%c%s", s_base_path, SEPERATOR, RC_NAME);
#endif
  f = fopen(fname, "w");
  if(f == 0) {
    printf("can't open %s ", fname);
    perror("for writing");
    free(fname);
    return; /* can't write rc */
  }
  for(i = 0; i < si_count; i++)
    fprintf(f, "iset %s %d\n", si[i].name, *(si[i].value));
  for(i = 0; i < sf_count; i++)
    fprintf(f, "fset %s %.2f\n", sf[i].name, *(sf[i].value));
  printf("written settings to %s\n", fname);
  free(fname);
  fclose(f);
  // Note: On Android, fullscreen enforcement happens at load time to avoid
  // mutating user settings immediately after saving.
}
