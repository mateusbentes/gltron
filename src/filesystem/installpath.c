/* Implementation of goto_installpath for all platforms */
#include "filesystem/nebu_filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <direct.h>
#define PATH_SEP '\\'
#define CHDIR _chdir
#else
#include <unistd.h>
#define PATH_SEP '/'
#define CHDIR chdir
#endif

/* Implementation of goto_installpath */
void goto_installpath(const char *executable) {
    char *path;
    char *p;
    int result;
    
    path = strdup(executable);
    p = strrchr(path, PATH_SEP);
    if(p) {
        *p = 0;
        result = CHDIR(path);
        if (result != 0) {
            fprintf(stderr, "Warning: Failed to change directory to %s\n", path);
        }
    }
    free(path);
}
