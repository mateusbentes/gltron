#include "filesystem/path.h"
#include "filesystem/dirsetup.h"
#include "Nebu_filesystem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

/* Define this to use embedded scripts instead of external files */
#define USE_EMBEDDED_SCRIPTS 1

const char* getHome() {
  return getenv("HOME");
}

void dirSetup(const char *executable) {
    char *path;
    char *dir;
    
    printf("[debug] dirSetup called with executable: %s\n", executable);
    
    /* Get the directory of the executable */
    path = strdup(executable);
    dir = dirname(path);
    printf("[debug] Executable directory: %s\n", dir);
    
    /* Set up the base directory - this is where we'll look for assets */
    char base_dir[PATH_MAX];
    
    /* Use the executable directory as the base directory */
    strcpy(base_dir, dir);
    
    printf("[debug] Using base directory: %s\n", base_dir);
    
#if USE_EMBEDDED_SCRIPTS
    /* When using embedded scripts, we'll set up a virtual path */
    printf("[debug] Using embedded scripts\n");
    
    /* Set up virtual paths for embedded resources */
    nebu_FS_SetupPath(PATH_SCRIPTS, "embedded://scripts");
    
    /* Still set up paths for other resources that might not be embedded */
    char art_dir[PATH_MAX];
    char music_dir[PATH_MAX];
    char level_dir[PATH_MAX];
    char data_dir[PATH_MAX];
    char snap_dir[PATH_MAX];
    char pref_dir[PATH_MAX];
    
    /* Use subdirectories of the base directory */
    sprintf(art_dir, "%s/art", base_dir);
    sprintf(music_dir, "%s/music", base_dir);
    sprintf(level_dir, "%s/levels", base_dir);
    sprintf(data_dir, "%s/data", base_dir);
    sprintf(snap_dir, "%s/snapshots", base_dir);
    sprintf(pref_dir, "%s", base_dir);  /* Use base directory for preferences */
    
    /* Set the directories for non-script resources */
    nebu_FS_SetupPath(PATH_ART, art_dir);
    nebu_FS_SetupPath(PATH_MUSIC, music_dir);
    nebu_FS_SetupPath(PATH_LEVEL, level_dir);
    nebu_FS_SetupPath(PATH_DATA, data_dir);
    nebu_FS_SetupPath(PATH_SNAPSHOTS, snap_dir);
    nebu_FS_SetupPath(PATH_PREFERENCES, pref_dir);

#else
    /* Set up the directories */
    char scripts_dir[PATH_MAX];
    char art_dir[PATH_MAX];
    char music_dir[PATH_MAX];
    char level_dir[PATH_MAX];
    char data_dir[PATH_MAX];
    char snap_dir[PATH_MAX];
    char pref_dir[PATH_MAX];
    
    /* Use subdirectories of the base directory */
    sprintf(scripts_dir, "%s/scripts", base_dir);
    sprintf(art_dir, "%s/art", base_dir);
    sprintf(music_dir, "%s/music", base_dir);
    sprintf(level_dir, "%s/levels", base_dir);
    sprintf(data_dir, "%s/data", base_dir);
    sprintf(snap_dir, "%s/snapshots", base_dir);
    sprintf(pref_dir, "%s", base_dir);  /* Use base directory for preferences */
    
    /* Check if the directories exist */
    struct stat st;
    if (stat(scripts_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[warning] Scripts directory not found: %s\n", scripts_dir);
    }
    if (stat(art_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[warning] Art directory not found: %s\n", art_dir);
    }
    if (stat(music_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[warning] Music directory not found: %s\n", music_dir);
    }
    if (stat(level_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[warning] Level directory not found: %s\n", level_dir);
    }
    if (stat(data_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[warning] Data directory not found: %s\n", data_dir);
    }
    if (stat(snap_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "[warning] Snapshots directory not found: %s\n", snap_dir);
    }
    
    /* Set the directories */
    nebu_FS_SetupPath(PATH_SCRIPTS, scripts_dir);
    nebu_FS_SetupPath(PATH_ART, art_dir);
    nebu_FS_SetupPath(PATH_MUSIC, music_dir);
    nebu_FS_SetupPath(PATH_LEVEL, level_dir);
    nebu_FS_SetupPath(PATH_DATA, data_dir);
    nebu_FS_SetupPath(PATH_SNAPSHOTS, snap_dir);
    nebu_FS_SetupPath(PATH_PREFERENCES, pref_dir);
#endif
    
#ifdef LOCAL_DATA
    goto_installpath(executable);
#endif
    initDirectories();
    
    free(path);
}

