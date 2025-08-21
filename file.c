#include "gltron.h"
#include <string.h>

#ifdef ANDROID
#include <android/asset_manager.h>  // For Android asset management
#include <android/asset_manager_jni.h>  // For AAssetManager functions
#include <sys/stat.h>  // For mkdir
#include <sys/types.h>  // For mkdir
#endif

char* getFullPath(char *filename) {
#ifdef ANDROID
  // On Android, prefer APK assets using AAssetManager if available.
  extern char s_base_path[]; // from android_glue.c
  extern AAssetManager* g_android_asset_mgr; // Declare the external variable

  if (g_android_asset_mgr) {
    AAsset* asset = AAssetManager_open(g_android_asset_mgr, filename, AASSET_MODE_STREAMING);
    if (asset) {
      // Compose destination path under base path
      char tmpPath[512];
      snprintf(tmpPath, sizeof(tmpPath), "%s/%s", s_base_path, filename);

      // Ensure base directory exists (best-effort) and create subdirs as needed
      {
        char dirbuf[512];
        size_t len = strlen(s_base_path);
        if (len >= sizeof(dirbuf)) len = sizeof(dirbuf) - 1;
        memcpy(dirbuf, s_base_path, len);
        dirbuf[len] = '\0';

        // Create base path
        mkdir(dirbuf, 0700);

        // Append subdirectories from filename if any
        const char* p = filename;
        while (*p) {
          if (*p == '/' || *p == '\\') {
            // add component
            size_t cur = strlen(dirbuf);
            if (cur + 1 < sizeof(dirbuf)) {
              dirbuf[cur] = '/';
              dirbuf[cur+1] = '\0';
            }
            size_t remain = sizeof(dirbuf) - strlen(dirbuf) - 1;
            strncat(dirbuf, filename, remain);
            mkdir(dirbuf, 0700);
          }
          p++;
        }
      }

      // If target exists, skip re-extract (simple cache)
      FILE* existed = fopen(tmpPath, "rb");
      if (existed) {
        fclose(existed);
        AAsset_close(asset);
        char* ret = malloc(strlen(tmpPath)+1);
        strcpy(ret, tmpPath);
        printf("asset '%s' cached at '%s'\n", filename, ret);
        return ret;
      }

      // Create the file and write the asset data to it
      FILE* out = fopen(tmpPath, "wb");
      if (out) {
        const size_t bufSize = 4096;
        char buf[bufSize];
        int n;
        while ((n = AAsset_read(asset, buf, bufSize)) > 0) {
          fwrite(buf, 1, n, out);
        }
        fclose(out);
        AAsset_close(asset);
        char* ret = malloc(strlen(tmpPath)+1);
        strcpy(ret, tmpPath);
        printf("loaded asset '%s' to '%s'\n", filename, ret);
        return ret;
      }

      AAsset_close(asset);
    }
  }
#endif

  // For non-Android platforms or if asset loading failed
  char *path;
  FILE *fp = NULL;
  char *base;

  char *share1 = "/usr/share/games/gltron";
  char *share2 = "/usr/local/share/games/gltron";

  /* check a few directories for the files and */
  /* return the full path. */

  /* check: current directory, GLTRON_HOME, and, for UNIX only: */
  /* /usr/share/games/gltron and /usr/local/share/games/gltron */

  path = malloc(strlen(filename) + 1);
  sprintf(path, "%s", filename);

  printf("checking '%s'...", path);
  fp = fopen(path, "r");
  if(fp != 0) {
    fclose(fp);
    printf("ok\n");
    return path;
  }
  free(path);
  printf("unsuccessful\n");

  base = getenv("GLTRON_HOME");
  if(base != 0) {
    path = malloc(strlen(base) + 1 + strlen(filename) + 1);
    sprintf(path, "%s%c%s", base, SEPERATOR, filename);

    printf("checking '%s'...", path);
    fp = fopen(path, "r");
    if(fp != 0) {
      fclose(fp);
      printf("ok\n");
      return path;
    }
    free(path);
    printf("unsuccessful\n");
  }

  path = malloc(strlen(share1) + 1 + strlen(filename) + 1);
  sprintf(path, "%s%c%s", share1, SEPERATOR, filename);

  printf("checking '%s'", path);
  fp = fopen(path, "r");
  if(fp != 0) {
    printf("ok\n");
    fclose(fp);
    return path;
  }
  free(path);
  printf("unsuccessful\n");

  path = malloc(strlen(share2) + 1 + strlen(filename) + 1);
  sprintf(path, "%s%c%s", share2, SEPERATOR, filename);

  printf("checking '%s'", path);
  fp = fopen(path, "r");
  if(fp != 0) {
    fclose(fp);
    printf("ok\n");
    return path;
  }
  free(path);
  printf("unsuccessful\n");

  return 0;
}
