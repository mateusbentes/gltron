#ifndef NEBU_FILESYSTEM_H
#define NEBU_FILESYSTEM_H

#include "filesystem/path.h"

#ifdef __cplusplus
extern "C" {
#endif

// Filesystem functions for Android
void dirSetup(const char *executable);
const char* getDirectory(ePathLocation eLocation);
int checkDirectory(const char* path);
void createDirectory(const char* path);
void initDirectories(void);

// Directory types (matching ePathLocation enum)
#define PATH_PREFERENCES 0
#define PATH_SNAPSHOTS 1
#define PATH_DATA 2
#define PATH_SCRIPTS 3
#define PATH_MUSIC 4
#define PATH_ART 5
#define PATH_LEVEL 6

#ifdef __cplusplus
}
#endif

#endif /* NEBU_FILESYSTEM_H */

