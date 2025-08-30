#include "gltron.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#define BUFSIZE 100

// Forward declaration for Android base path
#ifdef ANDROID
extern char s_base_path[1024];
#endif

void initSettingData(char *filename) {
#ifdef ANDROID
  LOGI("initSettingData: begin");
  // Guard: ensure base path is initialized by android_main before proceeding
  if (!s_base_path[0]) {
    LOGI("initSettingData: s_base_path not set yet; deferring to android_glue init path");
    return;
  }
  // If filename is not an absolute path, defer to android_glue path (avoid creating in unknown CWD)
  if (filename && filename[0] != '/') {
    LOGI("initSettingData: non-absolute filename '%s' on Android; deferring to android_glue init path", filename);
    return;
  }
#endif

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
#ifdef ANDROID
  // Ensure base directory exists (store settings.txt directly under s_base_path)
  if (mkdir(s_base_path, 0755) != 0 && errno != EEXIST) {
    LOGI("initSettingData: failed to create base directory '%s': %s", s_base_path, strerror(errno));
    return;
  }
  snprintf(fullPath, sizeof(fullPath), "%s/%s", s_base_path, filename);
#else
  char *resolved = getFullPath(filename);
  if (resolved) {
    strncpy(fullPath, resolved, sizeof(fullPath) - 1);
    fullPath[sizeof(fullPath) - 1] = '\0';
    free(resolved);
  } else {
    strncpy(fullPath, filename, sizeof(fullPath) - 1);
    fullPath[sizeof(fullPath) - 1] = '\0';
  }
#endif

  // Check if file exists before attempting to open (unified approach)
  if (access(fullPath, F_OK) != 0) {
#ifdef ANDROID
    LOGI("initSettingData: settings file doesn't exist; deferring creation to android_glue (Android) or continuing with defaults");
#else
    printf("initSettingData: settings file doesn't exist at '%s'; continuing with defaults\n", fullPath);
#endif
    return;
  }

#ifdef ANDROID
  LOGI("initSettingData: opening settings file at: %s", fullPath);
#else
  printf("initSettingData: opening settings file at: %s\n", fullPath);
#endif

  f = fopen(fullPath, "r");
  if (!f) {
#ifdef ANDROID
    LOGI("initSettingData: fopen failed for %s: %s", fullPath, strerror(errno));
#else
    printf("initSettingData: fopen failed for %s: %s\n", fullPath, strerror(errno));
#endif
    return; // fall back to defaults
  }

  // Robust parsing with better error handling
  if (!fgets(buf, BUFSIZE, f)) {
    fclose(f);
    return;
  }
  if (sscanf(buf, "%d ", &n) != 1 || n < 0 || n > 1024) {
#ifdef ANDROID
    LOGI("initSettingData: invalid header format or count: %s", buf);
#else
    printf("initSettingData: invalid header format or count: %s\n", buf);
#endif
    fclose(f);
    return;
  }

  for(i = 0; i < n; i++) {
    if (!fgets(buf, BUFSIZE, f)) break;
    if (sscanf(buf, "%c%d ", &c, &count) != 2 || count < 0 || count > 1024) {
#ifdef ANDROID
      LOGI("initSettingData: invalid section format: %s", buf);
#else
      printf("initSettingData: invalid section format: %s\n", buf);
#endif
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
#ifdef ANDROID
      LOGI("initSettingData: parsed %d integer settings", si_count);
#else
      printf("initSettingData: parsed %d integer settings\n", si_count);
#endif
      break;
      
    case 'f': /* float */
      // Clean up existing allocation
      if (sf) { free(sf); sf = NULL; }
      sf_count = 0;
      
      sf = malloc(sizeof(struct settings_float) * (size_t)count);
      if (!sf) { 
#ifdef ANDROID
        LOGI("initSettingData: failed to allocate memory for float settings");
#else
        printf("initSettingData: failed to allocate memory for float settings\n");
#endif
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
#ifdef ANDROID
      LOGI("initSettingData: parsed %d float settings", sf_count);
#else
      printf("initSettingData: parsed %d float settings\n", sf_count);
#endif
      break;
      
    default:
#ifdef ANDROID
      LOGI("initSettingData: unrecognized type '%c'", c);
#else
      printf("initSettingData: unrecognized type '%c'\n", c);
#endif
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
#ifdef ANDROID
      LOGI("initSettingData: failed to allocate default integer settings");
#else
      printf("initSettingData: failed to allocate default integer settings\n");
#endif
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
#ifdef ANDROID
      LOGI("initSettingData: failed to allocate default float settings");
#else
      printf("initSettingData: failed to allocate default float settings\n");
#endif
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
#ifdef ANDROID
  LOGI("After initSettingData - show_wall: %d, show_floor_texture: %d", 
       game->settings->show_wall, game->settings->show_floor_texture);
#else
  printf("After initSettingData - show_wall: %d, show_floor_texture: %d\n", 
         game->settings->show_wall, game->settings->show_floor_texture);
#endif

  /* go for .gltronrc (or whatever is defined in RC_NAME) to override defaults */
#ifndef ANDROID
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
  char *fname;
  int i;
  FILE* f;

#ifdef ANDROID
  // For Android, use internal storage base path directly and ensure it exists
  if (mkdir(s_base_path, 0755) != 0 && errno != EEXIST) {
    LOGI("saveSettings: failed to create base directory '%s': %s", s_base_path, strerror(errno));
    return;
  }

  // Construct full path to settings file directly under base path
  fname = malloc(strlen(s_base_path) + 1 + strlen(RC_NAME) + 1);
  if (!fname) {
    LOGI("saveSettings: failed to allocate memory for filename");
    return;
  }
  sprintf(fname, "%s/%s", s_base_path, RC_NAME);
#else
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
#endif

#ifdef ANDROID
  LOGI("saveSettings: saving to %s", fname);
#else
  printf("saveSettings: saving to %s\n", fname);
#endif

  f = fopen(fname, "w");
  if(f == 0) {
#ifdef ANDROID
    LOGI("saveSettings: can't open %s for writing: %s", fname, strerror(errno));
#else
    printf("saveSettings: can't open %s for writing: %s\n", fname, strerror(errno));
#endif
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

#ifdef ANDROID
  LOGI("saveSettings: written settings to %s", fname);
#else
  printf("saveSettings: written settings to %s\n", fname);
#endif

  free(fname);
  fclose(f);
  // Note: On Android, fullscreen enforcement happens at load time to avoid
  // mutating user settings immediately after saving.
}
