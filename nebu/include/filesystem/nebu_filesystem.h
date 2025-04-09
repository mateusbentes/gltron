#ifndef NEBU_FILESYSTEM_H
#define NEBU_FILESYSTEM_H

#include "base/nebu_util.h"
#include "filesystem/nebu_file_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Define platform-specific path separator */
#if defined(WIN32) || defined(_WIN32)
  #define PATH_SEPARATOR '\\'
#else
  #define PATH_SEPARATOR '/'
#endif

typedef struct {
	int tag;
	int nDirs;
	const char **directories;
} nebu_FS_PathConfig;

/* Clear all stored paths */
void nebu_FS_ClearAllPaths(void);

/* Set up a path for a specific resource type */
void nebu_FS_SetupPath(int type, const char *path);

/* Legacy path setup function */
void nebu_FS_SetupPath_WithDirs(int tag, int nDirs, const char **directories);

/* Get the path for a specific resource type */
const char* nebu_FS_GetPath(int type);

/* Legacy function - get full path for a file using the old path system */
char* nebu_FS_GetPath_WithFilename(int tag, const char *filename);

/* Test if a file exists */
int nebu_FS_Test(const char *path);

/* Create a directory */
int nebu_FS_MakeDir(const char *path);

/* Check if a path is a directory */
int nebu_FS_IsDir(const char *path);

/* Get the full path for a file */
char* nebu_FS_GetFullPath(int type, const char *filename);

/* Get the directory part of a path */
char* nebu_FS_GetDirname(const char *path);

/* Get the filename part of a path */
char* nebu_FS_GetBasename(const char *path);

/* Create all directories in a path */
int nebu_FS_MakeDirRecursive(const char *path);

/* Change to the installation directory (findpath.c, GPL'd code) */
void goto_installpath(const char *executable);

/* Read directory contents with optional prefix filter */
nebu_List* readDirectoryContents(const char *dirname, const char *prefix);

#ifdef __cplusplus
}
#endif

#endif /* NEBU_FILESYSTEM_H */
