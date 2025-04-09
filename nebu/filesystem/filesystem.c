#include "filesystem/nebu_filesystem.h"
#include "base/nebu_util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base/nebu_assert.h"

#include "base/nebu_debug_memory.h"

#ifdef WIN32
#include <direct.h>
#include <windows.h>
#define PATH_SEPARATOR '\\'
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#define PATH_SEPARATOR '/'
#endif

/* Maximum number of paths we can store */
#define MAX_PATHS 32

/* Path storage */
static char* paths[MAX_PATHS] = { NULL };
static int numPaths = 0;

int nebu_FS_Test(const char *path) {
  FILE *f = fopen(path, "r");
  if(f) {
    fclose(f);
    return 1;
  }
  return 0;
}

/* Implementation of goto_installpath for non-Windows platforms */
#ifndef WIN32
void goto_installpath(const char *executable) {
    char *path;
    char *p;
    int result;
    
    path = strdup(executable);
    p = strrchr(path, PATH_SEPARATOR);
    if(p) {
        *p = 0;
        result = chdir(path);
        if (result != 0) {
            fprintf(stderr, "Warning: Failed to change directory to %s\n", path);
        }
    }
    free(path);
}
#endif

nebu_List *path_list = NULL;

void nebu_FS_ClearAllPaths(void)
{
    int i;
    for(i = 0; i < numPaths; i++) {
        if(paths[i]) {
            free(paths[i]);
            paths[i] = NULL;
        }
    }
    numPaths = 0;
    
    /* Also clear the legacy path list if it exists */
    nebu_List *p;
    if(!path_list)
        return;

    for(p = path_list; p->next; p = p->next)
    {
        free(p->data);
    }
    nebu_List_Free(path_list);
    path_list = NULL;
}

/* Set up a path for a specific resource type */
void nebu_FS_SetupPath(int type, const char *path)
{
    if(type >= 0 && type < MAX_PATHS) {
        if(paths[type]) {
            free(paths[type]);
        }
        paths[type] = strdup(path);
        if(type >= numPaths) {
            numPaths = type + 1;
        }
    }
}

/* Legacy path setup function - maintains compatibility with old code */
void nebu_FS_SetupPath_WithDirs(int tag, int nDirs, const char **directories)
{
    nebu_List *p;

    if(path_list == NULL)
    {
        path_list = (nebu_List*) malloc(sizeof(nebu_List));
        path_list->next = NULL;
    }

    for(p = path_list; p->next != 0; p = p->next)
    {
        nebu_FS_PathConfig *pConfig = (nebu_FS_PathConfig*) p->data;
        if(pConfig->tag == tag)
        {
            // replace directories
            // TODO: consider memory management
            pConfig->nDirs = nDirs;
            pConfig->directories = directories;
            return;
        }
    }
    p->next = malloc(sizeof(nebu_List));
    p->next->next = NULL;
    p->data = malloc(sizeof(nebu_FS_PathConfig));
    // append directories
    {
        nebu_FS_PathConfig *pConfig = (nebu_FS_PathConfig*) p->data;
        pConfig->tag = tag;
        pConfig->nDirs = nDirs;
        pConfig->directories = directories;
    }
}

/* Get the path for a specific resource type */
const char* nebu_FS_GetPath(int type)
{
    if(type >= 0 && type < numPaths && paths[type]) {
        return paths[type];
    }
    return NULL;
}

/* Legacy function - get full path for a file using the old path system */
char* nebu_FS_GetPath_WithFilename(int tag, const char *filename)
{
    /* First try the new path system */
    if(tag >= 0 && tag < numPaths && paths[tag]) {
        int length = strlen(paths[tag]) + 1 /* separator */ + strlen(filename) + 1 /* terminator */;
        char *path = (char*) malloc(length);
        sprintf(path, "%s%c%s", paths[tag], PATH_SEPARATOR, filename);
        if(nebu_FS_Test(path))
            return path;
        free(path);
    }
    
    /* Fall back to the legacy path system */
    nebu_List *p;
    for(p = path_list; p && p->next != 0; p = p->next)
    {
        nebu_FS_PathConfig *pConfig = (nebu_FS_PathConfig*) p->data;
        if(pConfig->tag == tag)
        {
            int i;
            for(i = 0; i < pConfig->nDirs; i++)
            {
                int length = strlen(pConfig->directories[i]) + 1 /* separator */ + strlen(filename) + 1 /* terminator */;
                char *path = (char*) malloc(length);
                sprintf(path, "%s%c%s", pConfig->directories[i], PATH_SEPARATOR, filename);
                if(nebu_FS_Test(path))
                    return path;
                else
                    free(path);
            }
        }
    }
    return NULL;
}

/* Create a directory */
int nebu_FS_MakeDir(const char *path)
{
#ifdef WIN32
    return _mkdir(path) == 0 ? 1 : 0;
#else
    return mkdir(path, 0755) == 0 ? 1 : 0;
#endif
}

/* Check if a path is a directory */
int nebu_FS_IsDir(const char *path)
{
#ifdef WIN32
    /* Windows-specific directory check */
    DWORD attributes = GetFileAttributes(path);
    if (attributes != INVALID_FILE_ATTRIBUTES) {
        return (attributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
    }
    return 0;
#else
    /* POSIX directory check */
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 1 : 0;
    }
    return 0;
#endif
}

/* Get the full path for a file */
char* nebu_FS_GetFullPath(int type, const char *filename)
{
    if (type >= 0 && type < numPaths && paths[type]) {
        int length = strlen(paths[type]) + 1 /* separator */ + strlen(filename) + 1 /* terminator */;
        char *path = (char*) malloc(length);
        sprintf(path, "%s%c%s", paths[type], PATH_SEPARATOR, filename);
        return path;
    }
    return NULL;
}

/* Get the directory part of a path */
char* nebu_FS_GetDirname(const char *path)
{
    char *dirname = strdup(path);
    char *p = strrchr(dirname, PATH_SEPARATOR);
    if (p) {
        *p = '\0';
        return dirname;
    }
    free(dirname);
    return strdup(".");
}

/* Get the filename part of a path */
char* nebu_FS_GetBasename(const char *path)
{
    const char *p = strrchr(path, PATH_SEPARATOR);
    if (p) {
        return strdup(p + 1);
    }
    return strdup(path);
}

/* Create all directories in a path */
int nebu_FS_MakeDirRecursive(const char *path)
{
    char *tmp = strdup(path);
    char *p = tmp;
    int status = 0;
    
    /* Skip leading slashes */
    if (*p == PATH_SEPARATOR) {
        p++;
    }
    
    while (*p) {
        if (*p == PATH_SEPARATOR) {
            *p = 0;
            if (!nebu_FS_IsDir(tmp)) {
                status = nebu_FS_MakeDir(tmp);
                if (!status) {
                    free(tmp);
                    return 0;
                }
            }
            *p = PATH_SEPARATOR;
        }
        p++;
    }
    
    /* Create the final directory */
    if (!nebu_FS_IsDir(tmp)) {
        status = nebu_FS_MakeDir(tmp);
    }
    
    free(tmp);
    return status;
}

