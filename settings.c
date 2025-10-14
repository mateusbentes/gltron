#include "gltron.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define BUFSIZE 100

// Forward declaration for Android base path

void initSettingData(char *filename) {
  FILE *f = NULL;
  int n = 0, i, count = 0, j;
  char buf[BUFSIZE];
  char c = 0;

  if (!filename || filename[0] == '\0') {
#ifdef ANDROID
    LOGI("initSettingData: invalid filename");
#else
    printf("initSettingData: invalid filename\n");
#endif
    return;
  }

  // Construct full path (similar logic for both platforms)
  char fullPath[1024];
  char *resolved = getFullPath(filename);
  if (resolved) {
    strncpy(fullPath, resolved, sizeof(fullPath) - 1);
    fullPath[sizeof(fullPath) - 1] = '\0';
    free(resolved);
  } else {
    strncpy(fullPath, filename, sizeof(fullPath) - 1);
    fullPath[sizeof(fullPath) - 1] = '\0';
  }
  // Check if file exists before attempting to open (unified approach)
  if (access(fullPath, F_OK) != 0) {
    printf("initSettingData: settings file doesn't exist at '%s'; continuing with defaults\n", fullPath);
    return;
  }

#ifdef ANDROID
  LOGI("initSettingData: opening settings file at: %s", fullPath);
#else
  printf("initSettingData: opening settings file at: %s\n", fullPath);
#endif

  f = fopen(fullPath, "r");
  if (!f) {
    printf("initSettingData: fopen failed for %s: %s\n", fullPath, strerror(errno));
    return; // fall back to defaults
  }

  // Robust parsing with better error handling
  if (!fgets(buf, BUFSIZE, f)) {
    fclose(f);
    return;
  }
  if (sscanf(buf, "%d ", &n) != 1 || n < 0 || n > 1024) {
    printf("initSettingData: invalid header format or count: %s\n", buf);
    fclose(f);
    return;
  }

  for(i = 0; i < n; i++) {
    if (!fgets(buf, BUFSIZE, f)) break;
    if (sscanf(buf, "%c%d ", &c, &count) != 2 || count < 0 || count > 1024) {
      printf("initSettingData: invalid section format: %s\n", buf);
      continue;
    }
    switch(c) {
    case 'i': /* it's int */
      // Clean up existing allocation
      if (si) { free(si); si = NULL; }
      si_count = 0;
      
      si = malloc(sizeof(struct settings_int) * (size_t)count);
      if (!si) { 
#ifdef ANDROID
        LOGI("initSettingData: failed to allocate memory for integer settings");
#else
        printf("initSettingData: failed to allocate memory for integer settings\n");
#endif
        fclose(f); 
        return; 
      }
      si_count = count;
      
      for(j = 0; j < count; j++) {
        if (!fgets(buf, BUFSIZE, f)) { 
          si_count = j; 
          break; 
        }
        buf[BUFSIZE - 1] = 0;
        // Read a token into name (max 31 chars + NUL)
        if (sscanf(buf, "%31s", (si + j)->name) != 1) {
          (si + j)->name[0] = '\0';
        }
      }
      printf("initSettingData: parsed %d integer settings\n", si_count);
      break;
      
    case 'f': /* float */
      // Clean up existing allocation
      if (sf) { free(sf); sf = NULL; }
      sf_count = 0;
      
      sf = malloc(sizeof(struct settings_float) * (size_t)count);
      if (!sf) { 
        printf("initSettingData: failed to allocate memory for float settings\n");
        fclose(f); 
        return; 
      }
      sf_count = count;
      
      for(j = 0; j < count; j++) {
        if (!fgets(buf, BUFSIZE, f)) { 
          sf_count = j; 
          break; 
        }
        buf[BUFSIZE - 1] = 0;
        if (sscanf(buf, "%31s", (sf + j)->name) != 1) {
          (sf + j)->name[0] = '\0';
        }
      }
      printf("initSettingData: parsed %d float settings\n", sf_count);
      break;
      
    default:
      printf("initSettingData: unrecognized type '%c'\n", c);
      // Skip unknown sections gracefully
      for(j = 0; j < count; j++) {
        if (!fgets(buf, BUFSIZE, f)) break;
      }
      break;
    }
  }

  fclose(f);

  if (!game || !game->settings) return;

  // Ensure arrays are allocated to expected minimal sizes to avoid later deref
  if (!si || si_count < 28) {
    if (si) free(si);
    si = calloc(28, sizeof(struct settings_int));
    if (!si) {
      printf("initSettingData: failed to allocate default integer settings\n");
      return;
    }
    si_count = 28;
    // Initialize names to match defaults if parsing failed
    const char* names_int[28] = {
      "show_help","show_fps","show_wall","show_glow","show_2d","show_alpha",
      "show_floor_texture","line_spacing","erase_crashed","fast_finish",
      "fov","width","height","show_ai_status","camType","display_type",
      "playSound","show_model","ai_player1","ai_player2","ai_player3",
      "ai_player4","show_crash_texture","turn_cycle","mouse_warp",
      "sound_driver","input_mode","fullscreen"
    };
    for (int k = 0; k < 28; ++k) {
      strncpy(si[k].name, names_int[k], sizeof(si[k].name)-1);
      si[k].name[sizeof(si[k].name)-1] = '\0';
    }
  }
  
  if (!sf || sf_count < 1) {
    if (sf) free(sf);
    sf = calloc(1, sizeof(struct settings_float));
    if (!sf) {
      printf("initSettingData: failed to allocate default float settings\n");
      return;
    }
    sf_count = 1;
    strncpy(sf[0].name, "speed", sizeof(sf[0].name)-1);
    sf[0].name[sizeof(sf[0].name)-1] = '\0';
  }

  // Bind pointers (same for both platforms)
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
  if (!game->settings) {
    printf("initMainGameSettings: failed to allocate settings structure\n");
    return;
  }
  memset(game->settings, 0, sizeof(Settings));

  /* initialize defaults first, then load names/bindings from file */
  game->pauseflag = 0;

  // Set all defaults (same for both platforms)
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
  game->settings->joystick_deadzone = 0.15f; /* 15% deadzone */
  game->settings->joystick_enabled = 1; /* Enable joystick by default */
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

  /* Debug: Print current values after initSettingData */
  printf("After initSettingData - show_wall: %d, show_floor_texture: %d\n", 
         game->settings->show_wall, game->settings->show_floor_texture);

  /* go for .gltronrc (or whatever is defined in RC_NAME) to override defaults */
  home = getenv(HOMEVAR);
  if(home == 0) /* evaluate homedir */ {
    fname = malloc(strlen(CURRENT_DIR) + strlen(RC_NAME) + 2);
    if (!fname) {
      printf("initMainGameSettings: failed to allocate memory for filename\n");
      return;
    }
    sprintf(fname, "%s%c%s", CURRENT_DIR, SEPERATOR, RC_NAME);
  }
  else {
    fname = malloc(strlen(home) + strlen(RC_NAME) + 2);
    if (!fname) {
      printf("initMainGameSettings: failed to allocate memory for filename\n");
      return;
    }
    sprintf(fname, "%s%c%s", home, SEPERATOR, RC_NAME);
  }
  
  // Check if file exists before opening (like Android approach)
  if (access(fname, F_OK) != 0) {
    printf("no %s found - using defaults\n", fname);
    free(fname);
    return; /* no rc exists */
  }
  
  f = fopen(fname, "r");
  if(f == 0) {
    printf("failed to open %s: %s - using defaults\n", fname, strerror(errno));
    free(fname);
    return; /* can't read rc, use defaults */
  }
  
  printf("loading settings from %s\n", fname);
  
  while(fgets(buf, sizeof(buf), f)) {
    /* process rc-file */
    if(strstr(buf, "iset") == buf) {
      /* linear search through settings */
      /* first: integer */
      for(i = 0; i < si_count; i++) {
        sprintf(expbuf, "iset %s ", si[i].name);
        if(strstr(buf, expbuf) == buf) {
          if (sscanf(buf + strlen(expbuf), "%d ", si[i].value) == 1) {
            printf("loaded setting: %s = %d\n", si[i].name, *(si[i].value));
          }
          break;
        }
      }
    } else if(strstr(buf, "fset") == buf) {
      for(i = 0; i < sf_count; i++) {
        sprintf(expbuf, "fset %s ", sf[i].name);
        if(strstr(buf, expbuf) == buf) {
          if (sscanf(buf + strlen(expbuf), "%f ", sf[i].value) == 1) {
            printf("loaded setting: %s = %.2f\n", sf[i].name, *(sf[i].value));
          }
          break;
        }
      }
    }
  }
  free(fname);
  fclose(f);
  
  // Debug: Print floor texture setting after loading
  printf("Final show_wall: %d, show_floor_texture: %d\n", 
         game->settings->show_wall, game->settings->show_floor_texture);
  
}

void saveSettings() {
  char *fname;
  int i;
  FILE* f;

  char *home = getenv(HOMEVAR);
  if(home == 0) /* evaluate homedir */ {
    fname = malloc(strlen(CURRENT_DIR) + strlen(RC_NAME) + 2);
    if (!fname) {
      printf("saveSettings: failed to allocate memory for filename\n");
      return;
    }
    sprintf(fname, "%s%c%s", CURRENT_DIR, SEPERATOR, RC_NAME);
  }
  else {
    fname = malloc(strlen(home) + strlen(RC_NAME) + 2);
    if (!fname) {
      printf("saveSettings: failed to allocate memory for filename\n");
      return;
    }
    sprintf(fname, "%s%c%s", home, SEPERATOR, RC_NAME);
  }

  printf("saveSettings: saving to %s\n", fname);

  f = fopen(fname, "w");
  if(f == 0) {
    printf("saveSettings: can't open %s for writing: %s\n", fname, strerror(errno));
    free(fname);
    return; /* can't write rc */
  }

  // Write header with counts
  fprintf(f, "%d\n", 2); // Number of sections (int and float)

  // Write integer settings
  fprintf(f, "i %d\n", si_count);
  for(i = 0; i < si_count; i++) {
    fprintf(f, "%s\n", si[i].name);
  }

  // Write float settings
  fprintf(f, "f %d\n", sf_count);
  for(i = 0; i < sf_count; i++) {
    fprintf(f, "%s\n", sf[i].name);
  }

  // Write actual values
  for(i = 0; i < si_count; i++)
    fprintf(f, "iset %s %d\n", si[i].name, *(si[i].value));
  for(i = 0; i < sf_count; i++)
    fprintf(f, "fset %s %.2f\n", sf[i].name, *(sf[i].value));

  printf("saveSettings: written settings to %s\n", fname);
  
  free(fname);
  fclose(f);
  // Note: On Android, fullscreen enforcement happens at load time to avoid
  // mutating user settings immediately after saving.
}
